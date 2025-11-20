// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "rnode_core.h"
#include <applibs/log.h>
#include <applibs/uart.h>
#include <string.h>
#include <unistd.h>

static int radio_uart_fd = -1;
static uint32_t rx_packet_count = 0;
static uint32_t tx_packet_count = 0;

bool RNode_Initialize(int uart_fd) {
    Log_Debug("RNode: Initializing core functionality\n");
    
    if (uart_fd < 0) {
        Log_Debug("ERROR: Invalid UART file descriptor\n");
        return false;
    }
    
    radio_uart_fd = uart_fd;
    rx_packet_count = 0;
    tx_packet_count = 0;
    
    // TODO: Initialize LoRa module via UART
    // Send initialization commands to the LoRa transceiver
    
    Log_Debug("RNode: Core initialization complete\n");
    return true;
}

void RNode_Cleanup(void) {
    Log_Debug("RNode: Cleaning up\n");
    radio_uart_fd = -1;
}

void RNode_ProcessPacket(const uint8_t *data, size_t length, int rssi) {
    if (data == NULL || length == 0) {
        return;
    }
    
    // CRITICAL SECURITY REQUIREMENT:
    // Process packet locally ONLY
    // DO NOT forward Reticulum payload data to HLApp via ICM
    
    rx_packet_count++;
    
    Log_Debug("RNode: Processed packet (len=%zu, rssi=%d)\n", length, rssi);
    
    // Handle packet according to RNode protocol
    // This would include KISS framing, TNC mode operations, etc.
    // Implementation details depend on the RNode protocol specification
}

bool RNode_TransmitPacket(const uint8_t *data, size_t length) {
    if (radio_uart_fd < 0 || data == NULL || length == 0) {
        return false;
    }
    
    // Transmit packet via LoRa UART
    ssize_t written = UART_Write(radio_uart_fd, data, length);
    
    if (written == (ssize_t)length) {
        tx_packet_count++;
        Log_Debug("RNode: Transmitted packet (len=%zu)\n", length);
        return true;
    } else {
        Log_Debug("ERROR: Failed to transmit packet (written=%zd, expected=%zu)\n",
                  written, length);
        return false;
    }
}

void RNode_GetStatistics(uint32_t *rx_count, uint32_t *tx_count) {
    if (rx_count) *rx_count = rx_packet_count;
    if (tx_count) *tx_count = tx_packet_count;
}
