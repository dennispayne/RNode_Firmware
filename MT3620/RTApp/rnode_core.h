// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#ifndef RNODE_CORE_H
#define RNODE_CORE_H

#include <stdbool.h>
#include <stdint.h>

// Initialize RNode core functionality
bool RNode_Initialize(int uart_fd);

// Cleanup RNode resources
void RNode_Cleanup(void);

// Process incoming LoRa packet
// SECURITY: Packets are processed locally ONLY, never forwarded
void RNode_ProcessPacket(const uint8_t *data, size_t length, int rssi);

// Transmit packet via LoRa
bool RNode_TransmitPacket(const uint8_t *data, size_t length);

// Get current radio statistics
void RNode_GetStatistics(uint32_t *rx_count, uint32_t *tx_count);

#endif // RNODE_CORE_H
