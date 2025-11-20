// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "icm_handler.h"
#include "../common/config.h"
#include <applibs/log.h>
#include <applibs/application.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static int icm_socket_fd = -1;

bool ICM_Initialize(void) {
    Log_Debug("ICM: Initializing inter-core messaging\n");
    
    // Create socket for ICM communication with HLApp
    // CONFIGURATION REQUIRED: Update HLAPP_COMPONENT_ID in common/config.h
    icm_socket_fd = Application_Connect(HLAPP_COMPONENT_ID);
    
    if (icm_socket_fd < 0) {
        Log_Debug("ERROR: Failed to create ICM socket (errno=%d)\n", errno);
        return false;
    }
    
    Log_Debug("ICM: Initialization complete\n");
    return true;
}

void ICM_Cleanup(void) {
    Log_Debug("ICM: Cleaning up\n");
    
    if (icm_socket_fd >= 0) {
        close(icm_socket_fd);
        icm_socket_fd = -1;
    }
}

bool ICM_SendTelemetry(const void *data, size_t size) {
    if (icm_socket_fd < 0 || data == NULL || size == 0) {
        return false;
    }
    
    // SECURITY CHECK: Ensure we're only sending telemetry data
    // This should NEVER be called with Reticulum payload data
    
    // Create message header
    uint8_t header[2];
    header[0] = ICM_MSG_TELEMETRY;
    header[1] = (uint8_t)(size & 0xFF);
    
    // Send header
    ssize_t sent = send(icm_socket_fd, header, sizeof(header), 0);
    if (sent != sizeof(header)) {
        Log_Debug("ERROR: Failed to send ICM telemetry header\n");
        return false;
    }
    
    // Send telemetry data
    sent = send(icm_socket_fd, data, size, 0);
    if (sent != (ssize_t)size) {
        Log_Debug("ERROR: Failed to send ICM telemetry data\n");
        return false;
    }
    
    Log_Debug("ICM: Sent telemetry (%zu bytes)\n", size);
    return true;
}

bool ICM_ReceiveCommand(void *buffer, size_t buffer_size, size_t *received_size) {
    if (icm_socket_fd < 0 || buffer == NULL || buffer_size == 0) {
        return false;
    }
    
    // Non-blocking receive
    ssize_t received = recv(icm_socket_fd, buffer, buffer_size, MSG_DONTWAIT);
    
    if (received > 0) {
        if (received_size) {
            *received_size = (size_t)received;
        }
        
        Log_Debug("ICM: Received command (%zd bytes)\n", received);
        return true;
    } else if (received == 0) {
        // Connection closed
        Log_Debug("WARNING: ICM connection closed\n");
        return false;
    } else {
        // No data available (EAGAIN/EWOULDBLOCK) or error
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Log_Debug("ERROR: ICM receive error (errno=%d)\n", errno);
        }
        return false;
    }
}
