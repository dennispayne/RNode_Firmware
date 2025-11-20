// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

// Shared configuration definitions
// IMPORTANT: Update these values before deployment!

#ifndef CONFIG_H
#define CONFIG_H

// Component IDs for Inter-Core Messaging
// CONFIGURATION REQUIRED: Replace with actual UUIDs generated via uuidgen
#define RTAPP_COMPONENT_ID "RTAPP-COMPONENT-ID-HERE"
#define HLAPP_COMPONENT_ID "HLAPP-COMPONENT-ID-HERE"

// Azure IoT Hub configuration
// CONFIGURATION REQUIRED: Replace with your Azure IoT Hub details
#define AZURE_DPS_SCOPE_ID "YOUR-SCOPE-ID-HERE"

#endif // CONFIG_H
