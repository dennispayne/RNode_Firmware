// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "telemetry_validator.h"
#include <applibs/log.h>
#include <string.h>

// Expected telemetry message structure
typedef struct {
    uint32_t uptime_seconds;
    uint32_t packets_received;
    uint32_t packets_transmitted;
    int16_t last_rssi[5];
    uint8_t rssi_count;
} TelemetryMessage;

// CRITICAL SECURITY FUNCTION
// This function ensures that ONLY operational telemetry is forwarded to Azure
// and that Reticulum payload data is NEVER forwarded
bool Telemetry_Validate(const void *data, size_t size) {
    if (data == NULL) {
        Log_Debug("SECURITY: Null data pointer\n");
        return false;
    }
    
    // Check if size matches expected telemetry structure
    if (size != sizeof(TelemetryMessage)) {
        Log_Debug("SECURITY: Invalid telemetry size (expected=%zu, got=%zu)\n",
                  sizeof(TelemetryMessage), size);
        
        // If the size is significantly larger, it might be a payload leak attempt
        if (size > sizeof(TelemetryMessage) * 2) {
            Log_Debug("SECURITY ALERT: Suspiciously large message - possible payload!\n");
        }
        
        return false;
    }
    
    // Cast to telemetry structure for validation
    const TelemetryMessage *msg = (const TelemetryMessage *)data;
    
    // Validate field ranges to ensure this is legitimate telemetry
    
    // RSSI count should be 0-5
    if (msg->rssi_count > 5) {
        Log_Debug("SECURITY: Invalid RSSI count (%u)\n", msg->rssi_count);
        return false;
    }
    
    // RSSI values should be in valid range (-150 to 0 dBm)
    for (int i = 0; i < msg->rssi_count; i++) {
        if (msg->last_rssi[i] < -150 || msg->last_rssi[i] > 0) {
            Log_Debug("SECURITY: Invalid RSSI value (%d dBm)\n", msg->last_rssi[i]);
            return false;
        }
    }
    
    // Uptime should be reasonable (less than 10 years in seconds)
    const uint32_t max_uptime = 10 * 365 * 24 * 60 * 60; // 10 years
    if (msg->uptime_seconds > max_uptime) {
        Log_Debug("SECURITY: Invalid uptime (%u seconds)\n", msg->uptime_seconds);
        return false;
    }
    
    // Packet counts should be reasonable (not impossibly high)
    const uint32_t max_packets = 0xFFFFFF00; // Allow up to near max uint32
    if (msg->packets_received > max_packets || msg->packets_transmitted > max_packets) {
        Log_Debug("SECURITY: Suspicious packet counts (rx=%u, tx=%u)\n",
                  msg->packets_received, msg->packets_transmitted);
        return false;
    }
    
    // All checks passed - this appears to be valid telemetry
    Log_Debug("SECURITY: Telemetry validation passed\n");
    return true;
}
