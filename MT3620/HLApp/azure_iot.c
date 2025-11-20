// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "azure_iot.h"
#include "../common/config.h"
#include <applibs/log.h>
#include <applibs/networking.h>
#include <azureiot/iothub_device_client_ll.h>
#include <azureiot/iothub_client_options.h>
#include <azureiot/iothubtransportmqtt.h>
#include <azureiot/iothub_message.h>
#include <azureiot/azure_sphere_provisioning.h>
#include <string.h>

static IOTHUB_DEVICE_CLIENT_LL_HANDLE iothub_handle = NULL;
static bool connected_to_iothub = false;

// Command receive buffer
static uint8_t pending_command[256];
static size_t pending_command_size = 0;
static bool has_pending_command = false;

// Connection status callback
static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result,
                                     IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                                     void *userContextCallback) {
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        Log_Debug("Azure IoT: Connected to IoT Hub\n");
        connected_to_iothub = true;
    } else {
        Log_Debug("Azure IoT: Disconnected from IoT Hub (reason=%d)\n", reason);
        connected_to_iothub = false;
    }
}

// Message send confirmation callback
static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result,
                                     void *userContextCallback) {
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        Log_Debug("Azure IoT: Telemetry sent successfully\n");
    } else {
        Log_Debug("Azure IoT: Failed to send telemetry (result=%d)\n", result);
    }
}

// Cloud-to-Device message callback
static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(
    IOTHUB_MESSAGE_HANDLE message, void *userContextCallback) {
    
    const unsigned char *buffer;
    size_t size;
    
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) == IOTHUB_MESSAGE_OK) {
        if (size <= sizeof(pending_command)) {
            memcpy(pending_command, buffer, size);
            pending_command_size = size;
            has_pending_command = true;
            
            Log_Debug("Azure IoT: Received C2D message (%zu bytes)\n", size);
            return IOTHUBMESSAGE_ACCEPTED;
        } else {
            Log_Debug("Azure IoT: C2D message too large (%zu bytes)\n", size);
            return IOTHUBMESSAGE_REJECTED;
        }
    }
    
    return IOTHUBMESSAGE_ABANDONED;
}

bool Azure_Initialize(void) {
    Log_Debug("Azure IoT: Initializing connection to IoT Hub\n");
    
    // Use Azure Sphere DPS (Device Provisioning Service) for authentication
    // This provides passwordless, certificate-based authentication
    
    // CONFIGURATION REQUIRED: Update AZURE_DPS_SCOPE_ID in common/config.h
    AZURE_SPHERE_PROV_RETURN_VALUE prov_result =
        IoTHubDeviceClient_LL_CreateWithAzureSphereDeviceAuthProvisioning(
            "global.azure-devices-provisioning.net",  // DPS global endpoint
            AZURE_DPS_SCOPE_ID,                       // DPS ID Scope
            &iothub_handle);
    
    if (prov_result.result != AZURE_SPHERE_PROV_RESULT_OK) {
        Log_Debug("ERROR: Failed to create IoT Hub client (error=%d)\n",
                  prov_result.result);
        return false;
    }
    
    // Set connection status callback
    if (IoTHubDeviceClient_LL_SetConnectionStatusCallback(
            iothub_handle, ConnectionStatusCallback, NULL) != IOTHUB_CLIENT_OK) {
        Log_Debug("ERROR: Failed to set connection status callback\n");
        IoTHubDeviceClient_LL_Destroy(iothub_handle);
        iothub_handle = NULL;
        return false;
    }
    
    // Set C2D message callback
    if (IoTHubDeviceClient_LL_SetMessageCallback(
            iothub_handle, ReceiveMessageCallback, NULL) != IOTHUB_CLIENT_OK) {
        Log_Debug("ERROR: Failed to set message callback\n");
        IoTHubDeviceClient_LL_Destroy(iothub_handle);
        iothub_handle = NULL;
        return false;
    }
    
    // Enable diagnostic sampling (optional)
    bool diagnostic_sampling = true;
    if (IoTHubDeviceClient_LL_SetOption(
            iothub_handle, OPTION_DIAGNOSTIC_SAMPLING_PERCENTAGE,
            &diagnostic_sampling) != IOTHUB_CLIENT_OK) {
        Log_Debug("WARNING: Failed to enable diagnostic sampling\n");
    }
    
    Log_Debug("Azure IoT: Initialization complete\n");
    return true;
}

void Azure_Cleanup(void) {
    Log_Debug("Azure IoT: Cleaning up\n");
    
    if (iothub_handle != NULL) {
        IoTHubDeviceClient_LL_Destroy(iothub_handle);
        iothub_handle = NULL;
    }
    
    connected_to_iothub = false;
}

bool Azure_SendTelemetry(const void *data, size_t size) {
    if (iothub_handle == NULL || !connected_to_iothub) {
        return false;
    }
    
    if (data == NULL || size == 0) {
        return false;
    }
    
    // Create message
    IOTHUB_MESSAGE_HANDLE message = IoTHubMessage_CreateFromByteArray(
        (const unsigned char *)data, size);
    
    if (message == NULL) {
        Log_Debug("ERROR: Failed to create IoT Hub message\n");
        return false;
    }
    
    // Set message content type (optional)
    IoTHubMessage_SetContentTypeSystemProperty(message, "application/octet-stream");
    
    // Send message
    IOTHUB_CLIENT_RESULT result = IoTHubDeviceClient_LL_SendEventAsync(
        iothub_handle, message, SendConfirmationCallback, NULL);
    
    IoTHubMessage_Destroy(message);
    
    if (result != IOTHUB_CLIENT_OK) {
        Log_Debug("ERROR: Failed to send telemetry (result=%d)\n", result);
        return false;
    }
    
    return true;
}

bool Azure_ReceiveCommand(void *buffer, size_t buffer_size, size_t *received_size) {
    if (!has_pending_command) {
        return false;
    }
    
    if (buffer == NULL || buffer_size < pending_command_size) {
        return false;
    }
    
    memcpy(buffer, pending_command, pending_command_size);
    if (received_size) {
        *received_size = pending_command_size;
    }
    
    has_pending_command = false;
    return true;
}

void Azure_DoWork(void) {
    if (iothub_handle != NULL) {
        IoTHubDeviceClient_LL_DoWork(iothub_handle);
    }
}
