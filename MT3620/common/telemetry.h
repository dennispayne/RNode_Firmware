// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

// Shared telemetry structure definition
// This header is used by both RTApp and HLApp to ensure consistency

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

// Telemetry message structure
// Note: Using __attribute__((packed)) to ensure consistent size across cores
typedef struct __attribute__((packed)) {
    uint32_t uptime_seconds;         // Device uptime in seconds
    uint32_t packets_received;       // Total packets received
    uint32_t packets_transmitted;    // Total packets transmitted
    int16_t last_rssi[5];           // Last 5 RSSI values in dBm
    uint8_t rssi_count;             // Number of valid RSSI values (0-5)
} TelemetryData;

// Telemetry structure size (for validation)
#define TELEMETRY_SIZE sizeof(TelemetryData)

#endif // TELEMETRY_H
