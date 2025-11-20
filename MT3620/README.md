# RNode Firmware for Azure Sphere MT3620

This directory contains the implementation of RNode firmware for the **Seeed Studio Azure Sphere MT3620 Development Kit**.

## Architecture Overview

The MT3620 implementation uses a **split-function, secured architecture** with complete data separation between local radio traffic and cloud connectivity:

```
┌─────────────────────────────────────────────────────────┐
│                    MT3620 Device                        │
│                                                         │
│  ┌────────────────────┐       ┌────────────────────┐  │
│  │   RTApp (M4 Core)  │       │  HLApp (A7 Core)   │  │
│  │                    │       │                    │  │
│  │  - RNode Radio I/O │◄─ICM─►│  - Azure IoT Hub   │  │
│  │  - Local Processing│       │  - Device Mgmt     │  │
│  │  - NO Internet     │       │  - Telemetry Fwd   │  │
│  │  - NO Payload Fwd  │       │  - Config C2D      │  │
│  └─────────┬──────────┘       └──────────┬─────────┘  │
│            │                              │            │
│            │ LoRa Radio                   │ Network    │
└────────────┼──────────────────────────────┼────────────┘
             │                              │
             ▼                              ▼
        Radio Traffic                  Azure IoT Hub
      (Local Only)                    (Telemetry Only)
```

## Security Model

### RTApp (Real-Time Application - M4 Core)
- **Function:** Local RNode radio operations
- **Security Constraints:**
  - ✅ Direct LoRa radio control via UART
  - ✅ Local packet processing
  - ✅ Operational telemetry generation
  - ❌ NO internet access
  - ❌ NO Reticulum payload forwarding
  - ❌ NO cloud connectivity

### HLApp (High-Level Application - A7 Core)
- **Function:** Secure device management
- **Security Constraints:**
  - ✅ Azure IoT Hub connectivity (HTTPS/MQTT)
  - ✅ Device-to-Cloud (D2C) telemetry forwarding
  - ✅ Cloud-to-Device (C2D) configuration commands
  - ❌ NO Reticulum payload forwarding
  - ❌ ONLY operational telemetry, NEVER user data

### Inter-Core Messaging (ICM)
- **Function:** Management and control traffic between cores
- **Security Constraints:**
  - ✅ Operational telemetry (uptime, packet counts, RSSI)
  - ✅ Configuration commands (TX power, radio restart)
  - ❌ NO Reticulum payload data
  - ❌ Strict validation on all messages

## Directory Structure

```
MT3620/
├── RTApp/                  # Real-Time Application (M4 Core)
│   ├── CMakeLists.txt      # Build configuration
│   ├── app_manifest.json   # Capabilities and permissions
│   ├── main.c              # Main application entry
│   ├── rnode_core.c/h      # RNode radio logic
│   └── icm_handler.c/h     # ICM communication
│
├── HLApp/                  # High-Level Application (A7 Core)
│   ├── CMakeLists.txt      # Build configuration
│   ├── app_manifest.json   # Capabilities and permissions
│   ├── main.c              # Main application entry
│   ├── azure_iot.c/h       # Azure IoT Hub integration
│   ├── icm_handler.c/h     # ICM communication
│   └── telemetry_validator.c/h  # CRITICAL: Payload leak prevention
│
└── README.md               # This file
```

## Prerequisites

### Software Requirements
1. **Visual Studio 2019/2022** with Azure Sphere workload, or
2. **Visual Studio Code** with Azure Sphere extension
3. **Azure Sphere SDK** (v24.03 or later)
4. **CMake** (3.10 or later)
5. **Azure IoT Hub** instance

### Hardware Requirements
1. **Seeed Studio Azure Sphere MT3620 Development Kit**
2. **LoRa Module** with SPI/UART interface (SX1276/1278/1262/1268)
3. **USB Cable** for device connection

## Configuration Steps

### 1. Update Component IDs

Both applications need to know each other's Component IDs for ICM communication.

**Generate UUIDs for your deployment:**
```bash
# Generate RTApp Component ID
uuidgen

# Generate HLApp Component ID
uuidgen
```

**Update RTApp/app_manifest.json:**
```json
{
  "ComponentId": "YOUR-RTAPP-COMPONENT-ID",
  "AllowedApplicationConnections": [ "YOUR-HLAPP-COMPONENT-ID" ]
}
```

**Update HLApp/app_manifest.json:**
```json
{
  "ComponentId": "YOUR-HLAPP-COMPONENT-ID",
  "AllowedApplicationConnections": [ "YOUR-RTAPP-COMPONENT-ID" ]
}
```

### 2. Configure Azure IoT Hub

**Update HLApp/app_manifest.json:**
```json
{
  "AllowedConnections": [ "your-iothub-name.azure-devices.net" ],
  "DeviceAuthentication": "YOUR-TENANT-ID-HERE"
}
```

**Update HLApp/azure_iot.c:**
```c
AZURE_SPHERE_PROV_RETURN_VALUE prov_result =
    IoTHubDeviceClient_LL_CreateWithAzureSphereDeviceAuthProvisioning(
        "global.azure-devices-provisioning.net",
        "YOUR-DPS-SCOPE-ID-HERE",  // Update this
        &iothub_handle);
```

### 3. Configure Hardware Pins

**Update RTApp/app_manifest.json** to match your hardware connections:
```json
{
  "Uart": [ "ISU0" ],
  "Gpio": [ 8, 9, 0, 1, 2 ]
}
```

Modify the GPIO numbers to match your MT3620 pin assignments for:
- GPIO 8: RX LED
- GPIO 9: TX LED
- GPIO 0: LoRa CS (Chip Select)
- GPIO 1: LoRa RESET
- GPIO 2: LoRa DIO (Interrupt)

## Building the Firmware

### Using Visual Studio

1. Open the solution in Visual Studio
2. Set both RTApp and HLApp as startup projects
3. Build → Build Solution (Ctrl+Shift+B)
4. Deploy both applications to the device

### Using Command Line

```bash
# Build RTApp
cd MT3620/RTApp
cmake -B build -S . -G "Ninja"
cmake --build build

# Build HLApp
cd ../HLApp
cmake -B build -S . -G "Ninja"
cmake --build build

# Deploy both applications
azsphere device sideload deploy --imagepackage build/RNodeRTApp.imagepackage
azsphere device sideload deploy --imagepackage build/RNodeHLApp.imagepackage
```

## Deployment

### 1. Prepare Device

```bash
# Claim device to your Azure Sphere tenant
azsphere device claim

# Enable development mode
azsphere device enable-development

# Configure WiFi (for HLApp connectivity)
azsphere device wifi add --ssid "YOUR-SSID" --psk "YOUR-PASSWORD"
```

### 2. Deploy Applications

```bash
# Deploy RTApp (M4 Core)
azsphere device sideload deploy --imagepackage RTApp/build/RNodeRTApp.imagepackage

# Deploy HLApp (A7 Core)
azsphere device sideload deploy --imagepackage HLApp/build/RNodeHLApp.imagepackage
```

### 3. Verify Operation

```bash
# Monitor RTApp logs
azsphere device app show-status
azsphere device app start --component-id YOUR-RTAPP-COMPONENT-ID

# Monitor HLApp logs
azsphere device app show-status
azsphere device app start --component-id YOUR-HLAPP-COMPONENT-ID
```

## Testing & Validation

### Functional Tests

1. **Radio Operation:** Verify LoRa module communication
2. **Telemetry Flow:** Confirm telemetry reaches Azure IoT Hub
3. **Configuration Commands:** Test C2D commands to RTApp
4. **LED Indicators:** Check RX/TX LED operation

### Security Tests

1. **Payload Isolation:** Verify Reticulum payloads stay local
2. **Telemetry Validation:** Confirm only valid telemetry forwarded
3. **Network Isolation:** Verify RTApp has no network access
4. **ICM Security:** Validate message size and format checks

## Telemetry Format

The RTApp sends operational telemetry in the following format:

```c
typedef struct {
    uint32_t uptime_seconds;         // Device uptime
    uint32_t packets_received;       // Total RX packets
    uint32_t packets_transmitted;    // Total TX packets
    int16_t last_rssi[5];           // Last 5 RSSI values (-dBm)
    uint8_t rssi_count;             // Valid RSSI count (0-5)
} TelemetryData;
```

## Configuration Commands

The HLApp can send configuration commands via C2D messages:

| Command | Code | Parameters | Description |
|---------|------|------------|-------------|
| SET_TX_POWER | 0x01 | power (uint8) | Set LoRa TX power |
| RESTART_RADIO | 0x02 | - | Restart LoRa module |

## Security Considerations

### Critical Security Requirements

1. **Data Separation:** Reticulum payloads MUST remain local
2. **Telemetry Validation:** ALL ICM messages validated before forwarding
3. **Network Isolation:** RTApp has NO network capabilities
4. **Component Isolation:** Each app can only communicate with authorized partners

### Security Auditing

The HLApp logs all security events:
- Invalid telemetry format
- Unexpected message sizes
- Failed validation attempts

Monitor these logs regularly for security anomalies.

## Troubleshooting

### RTApp Issues

**Problem:** LoRa module not responding
- Check UART configuration (ISU0, 115200 baud)
- Verify GPIO pin assignments in manifest
- Check LoRa module power and connections

**Problem:** ICM connection failed
- Verify both apps are deployed
- Check Component IDs match in both manifests
- Ensure both apps are running

### HLApp Issues

**Problem:** Cannot connect to Azure IoT Hub
- Verify WiFi connectivity
- Check IoT Hub hostname in manifest
- Confirm DPS Scope ID is correct
- Verify tenant ID and device authentication

**Problem:** Telemetry validation failures
- Check telemetry structure size
- Verify RSSI values are in range
- Review security logs for details

## Performance Tuning

- **Telemetry Frequency:** Default 60 seconds (adjust in RTApp/main.c)
- **UART Buffer Size:** 2048 bytes (RTApp configuration)
- **Queue Size:** 4096 bytes for packet buffering
- **ICM Message Size:** Max 512 bytes for telemetry

## License

Copyright (C) 2024, RNode Firmware Contributors

This implementation is licensed under the GNU General Public License v3.0.

## Support

For issues specific to MT3620 support:
1. Check Azure Sphere SDK documentation
2. Review MT3620 hardware documentation
3. Consult RNode Firmware documentation at https://unsigned.io

## References

- [Azure Sphere MT3620 Datasheet](https://wiki.seeedstudio.com/Azure_Sphere_MT3620_Development_Kit/)
- [Azure Sphere Documentation](https://docs.microsoft.com/azure-sphere/)
- [RNode Firmware](https://github.com/markqvist/rnode_firmware)
- [Reticulum Network Stack](https://reticulum.network)
