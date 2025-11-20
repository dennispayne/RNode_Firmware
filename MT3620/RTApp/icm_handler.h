// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#ifndef ICM_HANDLER_H
#define ICM_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Message type identifiers
#define ICM_MSG_TELEMETRY 0x01
#define ICM_MSG_CONFIG    0x02

// Initialize ICM communication with HLApp
bool ICM_Initialize(void);

// Cleanup ICM resources
void ICM_Cleanup(void);

// Send operational telemetry to HLApp
// SECURITY: Only send NON-payload operational data
bool ICM_SendTelemetry(const void *data, size_t size);

// Receive configuration command from HLApp
bool ICM_ReceiveCommand(void *buffer, size_t buffer_size, size_t *received_size);

#endif // ICM_HANDLER_H
