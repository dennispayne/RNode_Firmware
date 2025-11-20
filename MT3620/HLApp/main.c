// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

// HLApp for MT3620 - High-Level Application on A7 Core
// This application manages device securely with Azure IoT Hub
// SECURITY: ONLY telemetry/configuration, NEVER Reticulum payloads

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include <applibs/log.h>
#include <applibs/networking.h>
#include <applibs/storage.h>
#include <applibs/application.h>

#include "azure_iot.h"
#include "icm_handler.h"
#include "telemetry_validator.h"

// Application state
static volatile sig_atomic_t terminationRequired = false;

// Termination handler
static void TerminationHandler(int signalNumber) {
    terminationRequired = true;
}

// Process telemetry from RTApp
static void ProcessTelemetryFromRTApp(void) {
    uint8_t buffer[512];
    size_t received_size;
    
    if (ICM_ReceiveTelemetry(buffer, sizeof(buffer), &received_size)) {
        // CRITICAL SECURITY CHECK:
        // Validate that this is operational telemetry, NOT Reticulum payload
        if (!Telemetry_Validate(buffer, received_size)) {
            Log_Debug("SECURITY ALERT: Invalid telemetry received - potential payload leak!\n");
            Log_Debug("Message size: %zu bytes\n", received_size);
            // Log the incident but DO NOT forward the data
            return;
        }
        
        // Telemetry is validated - safe to forward to Azure
        Log_Debug("HLApp: Received valid telemetry (%zu bytes)\n", received_size);
        
        // Forward to Azure IoT Hub as Device-to-Cloud message
        if (!Azure_SendTelemetry(buffer, received_size)) {
            Log_Debug("WARNING: Failed to send telemetry to Azure IoT Hub\n");
        }
    }
}

// Process cloud-to-device commands from Azure
static void ProcessCloudCommands(void) {
    uint8_t cmd_buffer[256];
    size_t cmd_size;
    
    if (Azure_ReceiveCommand(cmd_buffer, sizeof(cmd_buffer), &cmd_size)) {
        Log_Debug("HLApp: Received command from cloud (%zu bytes)\n", cmd_size);
        
        // Forward configuration command to RTApp via ICM
        if (!ICM_SendConfiguration(cmd_buffer, cmd_size)) {
            Log_Debug("WARNING: Failed to forward command to RTApp\n");
        }
    }
}

// Check network connectivity
static bool CheckNetworkReady(void) {
    bool network_ready = false;
    
    if (Networking_IsNetworkingReady(&network_ready) < 0) {
        Log_Debug("ERROR: Networking_IsNetworkingReady failed: %s (%d)\n",
                  strerror(errno), errno);
        return false;
    }
    
    return network_ready;
}

// Main application entry point
int main(int argc, char *argv[]) {
    Log_Debug("=== RNode HLApp Starting ===\n");
    Log_Debug("Security Model: Cloud connectivity for telemetry/config ONLY\n");
    Log_Debug("CRITICAL: Never forward Reticulum payload data to cloud\n");
    
    // Register termination handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);
    
    // Wait for network connectivity
    Log_Debug("HLApp: Waiting for network connectivity...\n");
    while (!CheckNetworkReady() && !terminationRequired) {
        sleep(1);
    }
    
    if (terminationRequired) {
        return 0;
    }
    
    Log_Debug("HLApp: Network ready\n");
    
    // Initialize ICM for communication with RTApp
    if (!ICM_Initialize()) {
        Log_Debug("FATAL: Failed to initialize ICM\n");
        return 1;
    }
    
    // Initialize Azure IoT Hub connection
    if (!Azure_Initialize()) {
        Log_Debug("FATAL: Failed to initialize Azure IoT Hub connection\n");
        ICM_Cleanup();
        return 1;
    }
    
    Log_Debug("HLApp: Initialization complete, entering main loop\n");
    
    // Main application loop
    const int sleep_interval_ms = 100;
    while (!terminationRequired) {
        // Process telemetry from RTApp
        ProcessTelemetryFromRTApp();
        
        // Process commands from Azure IoT Hub
        ProcessCloudCommands();
        
        // Let Azure SDK do its work
        Azure_DoWork();
        
        // Sleep to prevent busy-waiting
        struct timespec ts = {
            .tv_sec = 0,
            .tv_nsec = sleep_interval_ms * 1000000
        };
        nanosleep(&ts, NULL);
    }
    
    // Cleanup
    Log_Debug("HLApp: Shutting down...\n");
    Azure_Cleanup();
    ICM_Cleanup();
    
    Log_Debug("=== RNode HLApp Exiting ===\n");
    return 0;
}
