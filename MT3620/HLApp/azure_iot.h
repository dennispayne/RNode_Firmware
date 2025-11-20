// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#ifndef AZURE_IOT_H
#define AZURE_IOT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Initialize Azure IoT Hub connection
bool Azure_Initialize(void);

// Cleanup Azure IoT resources
void Azure_Cleanup(void);

// Send telemetry to Azure IoT Hub (Device-to-Cloud)
bool Azure_SendTelemetry(const void *data, size_t size);

// Receive configuration command from Azure IoT Hub (Cloud-to-Device)
bool Azure_ReceiveCommand(void *buffer, size_t buffer_size, size_t *received_size);

// Process Azure IoT SDK work items
void Azure_DoWork(void);

#endif // AZURE_IOT_H
