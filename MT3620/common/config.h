// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

// Shared configuration definitions
// IMPORTANT: Update these values before deployment!

#ifndef CONFIG_H
#define CONFIG_H

// Component IDs for Inter-Core Messaging
// CONFIGURATION REQUIRED: Replace with actual UUIDs generated via uuidgen
// Example: "12345678-1234-5678-90ab-cdef01234567"
#define RTAPP_COMPONENT_ID "RTAPP-COMPONENT-ID-HERE"
#define HLAPP_COMPONENT_ID "HLAPP-COMPONENT-ID-HERE"

// Azure IoT Hub configuration
// CONFIGURATION REQUIRED: Replace with your Azure IoT Hub DPS ID Scope
// Example: "0ne00ABC123"
#define AZURE_DPS_SCOPE_ID "YOUR-SCOPE-ID-HERE"

// Compilation check to prevent accidental deployment with placeholders
#ifdef MT3620_STRICT_CONFIG_CHECK
  #if defined(RTAPP_COMPONENT_ID) && (RTAPP_COMPONENT_ID[0] == 'R')
    #error "RTAPP_COMPONENT_ID must be configured with actual UUID before deployment"
  #endif
  #if defined(HLAPP_COMPONENT_ID) && (HLAPP_COMPONENT_ID[0] == 'H')
    #error "HLAPP_COMPONENT_ID must be configured with actual UUID before deployment"
  #endif
  #if defined(AZURE_DPS_SCOPE_ID) && (AZURE_DPS_SCOPE_ID[0] == 'Y')
    #error "AZURE_DPS_SCOPE_ID must be configured with actual DPS Scope ID before deployment"
  #endif
#endif

#endif // CONFIG_H
