# MT3620 Configuration Guide

This guide helps you configure the RNode MT3620 implementation for your specific deployment.

## Required Configuration Values

Before deploying to your MT3620 device, you must configure the following values:

### 1. Component IDs

Component IDs are unique identifiers for each application. They enable secure inter-core communication.

**Generate UUIDs:**
```bash
# On Linux/macOS:
uuidgen

# On Windows PowerShell:
[guid]::NewGuid()
```

Generate two UUIDs, one for each application.

**Example:**
```
RTApp Component ID: 12345678-1234-5678-90ab-cdef01234567
HLApp Component ID: 87654321-4321-8765-ba09-fedcba987654
```

### 2. Azure IoT Hub Details

You need the following information from your Azure IoT Hub setup:

- **IoT Hub Hostname:** `your-iothub-name.azure-devices.net`
- **DPS ID Scope:** From Azure IoT Hub Device Provisioning Service
- **Tenant ID:** Your Azure Sphere tenant ID

**Get Tenant ID:**
```bash
azsphere tenant list
```

## Configuration Steps

### Step 1: Update RTApp Manifest

Edit `MT3620/RTApp/app_manifest.json`:

```json
{
  "ComponentId": "REPLACE-WITH-RTAPP-UUID",
  "AllowedApplicationConnections": [ "REPLACE-WITH-HLAPP-UUID" ]
}
```

**Example:**
```json
{
  "ComponentId": "12345678-1234-5678-90ab-cdef01234567",
  "AllowedApplicationConnections": [ "87654321-4321-8765-ba09-fedcba987654" ]
}
```

### Step 2: Update HLApp Manifest

Edit `MT3620/HLApp/app_manifest.json`:

```json
{
  "ComponentId": "REPLACE-WITH-HLAPP-UUID",
  "AllowedApplicationConnections": [ "REPLACE-WITH-RTAPP-UUID" ],
  "AllowedConnections": [ "YOUR-IOTHUB-NAME.azure-devices.net" ],
  "DeviceAuthentication": "YOUR-TENANT-ID"
}
```

**Example:**
```json
{
  "ComponentId": "87654321-4321-8765-ba09-fedcba987654",
  "AllowedApplicationConnections": [ "12345678-1234-5678-90ab-cdef01234567" ],
  "AllowedConnections": [ "myrnode-iothub.azure-devices.net" ],
  "DeviceAuthentication": "99999999-aaaa-bbbb-cccc-dddddddddddd"
}
```

### Step 3: Update Shared Configuration

Edit `MT3620/common/config.h`:

```c
// Component IDs for Inter-Core Messaging
#define RTAPP_COMPONENT_ID "REPLACE-WITH-RTAPP-UUID"
#define HLAPP_COMPONENT_ID "REPLACE-WITH-HLAPP-UUID"

// Azure IoT Hub configuration
#define AZURE_DPS_SCOPE_ID "YOUR-SCOPE-ID-HERE"
```

Replace with your actual values:

```c
// Component IDs for Inter-Core Messaging
#define RTAPP_COMPONENT_ID "12345678-1234-5678-90ab-cdef01234567"
#define HLAPP_COMPONENT_ID "87654321-4321-8765-ba09-fedcba987654"

// Azure IoT Hub configuration
#define AZURE_DPS_SCOPE_ID "0ne00ABC123"
```

### Step 4: Configure Hardware Pins (Optional)

If your LoRa module is connected to different GPIOs, update the pin assignments in `MT3620/RTApp/app_manifest.json`:

```json
{
  "Gpio": [ 8, 9, 0, 1, 2 ]
}
```

Default pin mapping:
- GPIO 8: RX LED
- GPIO 9: TX LED
- GPIO 0: LoRa CS (Chip Select)
- GPIO 1: LoRa RESET
- GPIO 2: LoRa DIO (Interrupt)

Update ISU (I/O Subsystem Unit) if using a different UART:

```json
{
  "Uart": [ "ISU0" ]
}
```

Available ISUs: ISU0, ISU1, ISU2, ISU3

## Verification Checklist

Before building and deploying:

- [ ] Generated two unique UUIDs
- [ ] Updated RTApp ComponentId in app_manifest.json
- [ ] Updated RTApp AllowedApplicationConnections in app_manifest.json
- [ ] Updated HLApp ComponentId in app_manifest.json
- [ ] Updated HLApp AllowedApplicationConnections in app_manifest.json
- [ ] Updated HLApp AllowedConnections with IoT Hub hostname
- [ ] Updated HLApp DeviceAuthentication with Tenant ID
- [ ] Updated common/config.h with Component IDs
- [ ] Updated common/config.h with DPS Scope ID
- [ ] Verified GPIO pin assignments match hardware
- [ ] Verified UART ISU matches hardware connection

## Configuration Templates

### Quick Setup Script

Create a file `configure.sh`:

```bash
#!/bin/bash

# Configuration values
RTAPP_UUID="12345678-1234-5678-90ab-cdef01234567"
HLAPP_UUID="87654321-4321-8765-ba09-fedcba987654"
IOTHUB_NAME="myrnode-iothub.azure-devices.net"
TENANT_ID="99999999-aaaa-bbbb-cccc-dddddddddddd"
DPS_SCOPE="0ne00ABC123"

# Update RTApp manifest
sed -i "s/RTAPP-COMPONENT-ID-HERE/$RTAPP_UUID/g" MT3620/RTApp/app_manifest.json
sed -i "s/HLAPP-COMPONENT-ID-HERE/$HLAPP_UUID/g" MT3620/RTApp/app_manifest.json

# Update HLApp manifest
sed -i "s/HLAPP-COMPONENT-ID-HERE/$HLAPP_UUID/g" MT3620/HLApp/app_manifest.json
sed -i "s/RTAPP-COMPONENT-ID-HERE/$RTAPP_UUID/g" MT3620/HLApp/app_manifest.json
sed -i "s/YOUR-IOTHUB-NAME.azure-devices.net/$IOTHUB_NAME/g" MT3620/HLApp/app_manifest.json
sed -i "s/YOUR-TENANT-ID-HERE/$TENANT_ID/g" MT3620/HLApp/app_manifest.json

# Update common configuration
sed -i "s/RTAPP-COMPONENT-ID-HERE/$RTAPP_UUID/g" MT3620/common/config.h
sed -i "s/HLAPP-COMPONENT-ID-HERE/$HLAPP_UUID/g" MT3620/common/config.h
sed -i "s/YOUR-SCOPE-ID-HERE/$DPS_SCOPE/g" MT3620/common/config.h

echo "Configuration complete!"
```

Make it executable and run:
```bash
chmod +x configure.sh
./configure.sh
```

## Azure IoT Hub Setup

### Prerequisites

1. **Azure Subscription:** You need an active Azure subscription
2. **Azure CLI:** Install from https://docs.microsoft.com/cli/azure/install-azure-cli

### Create IoT Hub

```bash
# Login to Azure
az login

# Create resource group
az group create --name rnode-resources --location westus2

# Create IoT Hub (Free tier)
az iot hub create --name myrnode-iothub \
                  --resource-group rnode-resources \
                  --sku F1 \
                  --partition-count 2

# Create Device Provisioning Service
az iot dps create --name myrnode-dps \
                  --resource-group rnode-resources \
                  --location westus2

# Link DPS to IoT Hub
IOT_HUB_CONNECTION=$(az iot hub connection-string show \
                     --hub-name myrnode-iothub \
                     --query connectionString -o tsv)

az iot dps linked-hub create --dps-name myrnode-dps \
                             --resource-group rnode-resources \
                             --connection-string "$IOT_HUB_CONNECTION"

# Get DPS ID Scope
az iot dps show --name myrnode-dps \
                --resource-group rnode-resources \
                --query properties.idScope -o tsv
```

### Configure Azure Sphere Device

```bash
# Claim device to your tenant
azsphere device claim

# Get tenant ID
azsphere tenant list

# Configure device for DPS
azsphere device enable-cloud-test --dps-scope YOUR-DPS-SCOPE
```

## Testing Configuration

After configuration, verify before deployment:

### 1. Validate JSON Manifests

```bash
# Check RTApp manifest
cat MT3620/RTApp/app_manifest.json | jq .

# Check HLApp manifest
cat MT3620/HLApp/app_manifest.json | jq .
```

### 2. Verify No Placeholders

```bash
# Search for placeholder strings
grep -r "COMPONENT-ID-HERE" MT3620/
grep -r "YOUR-" MT3620/

# Should return no results
```

### 3. Build Test

```bash
cd MT3620
./build.sh
```

Should complete without errors.

## Common Configuration Issues

### Issue: ICM Connection Failed

**Symptom:** RTApp and HLApp cannot communicate

**Solution:**
1. Verify Component IDs match in both manifests
2. Check AllowedApplicationConnections lists correct partner ID
3. Ensure both apps are deployed and running

### Issue: Azure IoT Hub Connection Failed

**Symptom:** HLApp cannot connect to Azure

**Solution:**
1. Verify IoT Hub hostname is correct
2. Check DPS ID Scope is accurate
3. Confirm Tenant ID matches device tenant
4. Ensure device is claimed and configured

### Issue: Build Errors

**Symptom:** CMake or build fails

**Solution:**
1. Verify Azure Sphere SDK is installed
2. Check all placeholder values are replaced
3. Ensure manifest JSON is valid
4. Review CMakeLists.txt for syntax errors

## Security Notes

- **Never commit** configured files with real Component IDs to public repositories
- **Rotate** Component IDs if they are exposed
- **Use** Azure Key Vault for additional secrets management
- **Monitor** Azure IoT Hub logs for unauthorized access attempts

## Next Steps

After configuration:

1. Build both applications: `cd MT3620 && ./build.sh`
2. Deploy to device: See MT3620/README.md
3. Monitor logs: `azsphere device app show-log`
4. Verify telemetry: Check Azure IoT Hub

## Support

For configuration help:
- Review MT3620/README.md
- Check Azure Sphere documentation
- Consult RNode community resources
