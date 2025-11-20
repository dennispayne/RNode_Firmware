// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#ifndef ICM_HANDLER_H
#define ICM_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Initialize ICM communication with RTApp
bool ICM_Initialize(void);

// Cleanup ICM resources
void ICM_Cleanup(void);

// Receive operational telemetry from RTApp
// SECURITY: Validate that received data is telemetry, not payload
bool ICM_ReceiveTelemetry(void *buffer, size_t buffer_size, size_t *received_size);

// Send configuration command to RTApp
bool ICM_SendConfiguration(const void *data, size_t size);

#endif // ICM_HANDLER_H
