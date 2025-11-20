// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

// RTApp for MT3620 - Real-Time Application on M4 Core
// This application handles local RNode radio operations ONLY
// NO internet access, NO payload forwarding

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <applibs/log.h>
#include <applibs/uart.h>
#include <applibs/gpio.h>
#include <applibs/application.h>

#include "rnode_core.h"
#include "icm_handler.h"

// Application state
static bool running = true;
static int uart_fd = -1;
static int led_rx_fd = -1;
static int led_tx_fd = -1;

// Telemetry data structure
typedef struct {
    uint32_t uptime_seconds;
    uint32_t packets_received;
    uint32_t packets_transmitted;
    int16_t last_rssi[5];
    uint8_t rssi_count;
} TelemetryData;

static TelemetryData telemetry = {0};

// Initialize hardware
static bool InitializeHardware(void) {
    Log_Debug("RTApp: Initializing hardware...\n");
    
    // Open UART for LoRa module communication
    uart_fd = UART_Open(ISU0);
    if (uart_fd < 0) {
        Log_Debug("ERROR: Failed to open UART for LoRa module\n");
        return false;
    }
    
    // Configure UART (115200 8N1)
    UART_Config config = {
        .baudRate = 115200,
        .dataBits = UART_DataBits_Eight,
        .parity = UART_Parity_None,
        .stopBits = UART_StopBits_One,
        .flowControl = UART_FlowControl_None
    };
    
    if (UART_SetConfig(uart_fd, &config) < 0) {
        Log_Debug("ERROR: Failed to configure UART\n");
        return false;
    }
    
    // Initialize LED GPIOs
    led_rx_fd = GPIO_OpenAsOutput(8, GPIO_OutputMode_PushPull, GPIO_Value_High);
    led_tx_fd = GPIO_OpenAsOutput(9, GPIO_OutputMode_PushPull, GPIO_Value_High);
    
    if (led_rx_fd < 0 || led_tx_fd < 0) {
        Log_Debug("WARNING: Failed to open LED GPIOs\n");
    }
    
    Log_Debug("RTApp: Hardware initialized successfully\n");
    return true;
}

// Process RNode radio operations
static void ProcessRadioOperations(void) {
    // This is where the main RNode logic would run
    // For now, this is a placeholder that demonstrates the architecture
    
    // Read from LoRa UART
    uint8_t buffer[256];
    ssize_t bytes_read = UART_Read(uart_fd, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        // Process received LoRa packet
        // IMPORTANT: Packets are processed locally ONLY
        // NO forwarding to HLApp via ICM
        
        telemetry.packets_received++;
        
        // Blink RX LED
        if (led_rx_fd >= 0) {
            GPIO_SetValue(led_rx_fd, GPIO_Value_Low);
            struct timespec ts = { .tv_sec = 0, .tv_nsec = 50000000 }; // 50ms
            nanosleep(&ts, NULL);
            GPIO_SetValue(led_rx_fd, GPIO_Value_High);
        }
        
        // Update RSSI history (mock data for now)
        if (telemetry.rssi_count < 5) {
            telemetry.last_rssi[telemetry.rssi_count++] = -85; // Example RSSI
        } else {
            // Shift array and add new value
            for (int i = 0; i < 4; i++) {
                telemetry.last_rssi[i] = telemetry.last_rssi[i + 1];
            }
            telemetry.last_rssi[4] = -85;
        }
    }
}

// Send telemetry to HLApp via ICM
static void SendTelemetryUpdate(void) {
    // Increment uptime
    static uint32_t last_telemetry_time = 0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint32_t current_time = (uint32_t)now.tv_sec;
    
    // Send telemetry every 60 seconds
    if (current_time - last_telemetry_time >= 60) {
        telemetry.uptime_seconds = current_time;
        
        // Send ONLY operational telemetry, NEVER Reticulum payloads
        ICM_SendTelemetry(&telemetry, sizeof(TelemetryData));
        
        last_telemetry_time = current_time;
        Log_Debug("RTApp: Sent telemetry update (uptime=%u, rx=%u, tx=%u)\n",
                  telemetry.uptime_seconds,
                  telemetry.packets_received,
                  telemetry.packets_transmitted);
    }
}

// Handle configuration commands from HLApp
static void ProcessConfigurationCommands(void) {
    uint8_t cmd_buffer[128];
    size_t received_size;
    
    if (ICM_ReceiveCommand(cmd_buffer, sizeof(cmd_buffer), &received_size)) {
        // Process configuration command
        // Examples: SET_TX_POWER, RESTART_RADIO, etc.
        
        if (received_size >= 1) {
            uint8_t cmd_type = cmd_buffer[0];
            
            switch (cmd_type) {
                case 0x01: // SET_TX_POWER
                    if (received_size >= 2) {
                        uint8_t tx_power = cmd_buffer[1];
                        Log_Debug("RTApp: Setting TX power to %d\n", tx_power);
                        // Apply configuration to LoRa module
                    }
                    break;
                    
                case 0x02: // RESTART_RADIO
                    Log_Debug("RTApp: Restarting radio module\n");
                    // Restart LoRa module
                    break;
                    
                default:
                    Log_Debug("RTApp: Unknown command type: 0x%02X\n", cmd_type);
                    break;
            }
        }
    }
}

// Main application entry point
int main(void) {
    Log_Debug("=== RNode RTApp Starting ===\n");
    Log_Debug("Security Model: Local radio I/O ONLY, NO internet access\n");
    
    // Initialize hardware
    if (!InitializeHardware()) {
        Log_Debug("FATAL: Hardware initialization failed\n");
        return 1;
    }
    
    // Initialize ICM for communication with HLApp
    if (!ICM_Initialize()) {
        Log_Debug("FATAL: ICM initialization failed\n");
        return 1;
    }
    
    // Initialize RNode core functionality
    if (!RNode_Initialize(uart_fd)) {
        Log_Debug("FATAL: RNode initialization failed\n");
        return 1;
    }
    
    Log_Debug("RTApp: Initialization complete, entering main loop\n");
    
    // Main application loop
    while (running) {
        // Process local radio operations
        ProcessRadioOperations();
        
        // Handle configuration commands from HLApp
        ProcessConfigurationCommands();
        
        // Send telemetry updates periodically
        SendTelemetryUpdate();
        
        // Small delay to prevent busy-waiting
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 }; // 10ms
        nanosleep(&ts, NULL);
    }
    
    // Cleanup
    ICM_Cleanup();
    RNode_Cleanup();
    
    if (uart_fd >= 0) close(uart_fd);
    if (led_rx_fd >= 0) close(led_rx_fd);
    if (led_tx_fd >= 0) close(led_tx_fd);
    
    Log_Debug("=== RNode RTApp Exiting ===\n");
    return 0;
}
