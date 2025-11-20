// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#include "telemetry_validator.h"
#include "../common/telemetry.h"
#include <applibs/log.h>
#include <string.h>

// CRITICAL SECURITY FUNCTION
// This function ensures that ONLY operational telemetry is forwarded to Azure
// and that Reticulum payload data is NEVER forwarded
bool Telemetry_Validate(const void *data, size_t size) {
    if (data == NULL) {
        Log_Debug("SECURITY: Null data pointer\n");
        return false;
    }
    
    // Check if size matches expected telemetry structure
    if (size != TELEMETRY_SIZE) {
        Log_Debug("SECURITY: Invalid telemetry size (expected=%zu, got=%zu)\n",
                  TELEMETRY_SIZE, size);
        
        // If the size is significantly larger, it might be a payload leak attempt
        if (size > TELEMETRY_SIZE * 2) {
            Log_Debug("SECURITY ALERT: Suspiciously large message - possible payload!\n");
        }
        
        return false;
    }
    
    // Cast to telemetry structure for validation
    const TelemetryData *msg = (const TelemetryData *)data;
    
    // Validate field ranges to ensure this is legitimate telemetry
    
    // RSSI count should be 0-5
    if (msg->rssi_count > 5) {
        Log_Debug("SECURITY: Invalid RSSI count (%u)\n", msg->rssi_count);
        return false;
    }
    
    // RSSI values should be in valid range (-150 to 0 dBm)
    // Validate all array elements to prevent malicious data in unused slots
    for (int i = 0; i < 5; i++) {
        // For indices beyond rssi_count, value should be 0 (uninitialized)
        if (i < msg->rssi_count) {
            if (msg->last_rssi[i] < -150 || msg->last_rssi[i] > 0) {
                Log_Debug("SECURITY: Invalid RSSI value at index %d (%d dBm)\n",
                          i, msg->last_rssi[i]);
                return false;
            }
        } else {
            // Unused slots should be zero
            if (msg->last_rssi[i] != 0) {
                Log_Debug("SECURITY: Non-zero data in unused RSSI slot %d\n", i);
                return false;
            }
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
