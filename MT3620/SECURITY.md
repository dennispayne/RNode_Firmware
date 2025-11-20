# Security Architecture - RNode MT3620

## Overview

The RNode implementation for Azure Sphere MT3620 implements a **defense-in-depth security architecture** specifically designed to ensure complete separation between local radio traffic and cloud connectivity.

## Threat Model

### Protected Assets
1. **Reticulum Payload Data:** User communications transmitted via LoRa
2. **Device Integrity:** Prevention of unauthorized firmware modifications
3. **Cloud Credentials:** Azure IoT Hub authentication tokens

### Threats Addressed
1. **Data Exfiltration:** Reticulum payloads being forwarded to cloud
2. **Unauthorized Access:** Malicious actors accessing device or cloud
3. **Man-in-the-Middle:** Interception of cloud communications
4. **Code Injection:** Unauthorized code execution on device

## Security Layers

### Layer 1: Hardware Isolation (MT3620 Silicon)

The MT3620 provides hardware-enforced isolation:

```
┌────────────────────────────────────────┐
│          MT3620 Hardware              │
│                                        │
│  ┌──────────┐         ┌──────────┐   │
│  │ M4 Core  │         │ A7 Core  │   │
│  │ (RTApp)  │◄───────►│ (HLApp)  │   │
│  │          │   ICM   │          │   │
│  └────┬─────┘         └─────┬────┘   │
│       │                     │         │
│       │                     │         │
│  ┌────▼─────┐         ┌─────▼────┐   │
│  │   UART   │         │ Network  │   │
│  │ (LoRa)   │         │ Stack    │   │
│  └──────────┘         └──────────┘   │
└────────────────────────────────────────┘
```

**Enforcement:**
- M4 core has **NO network stack access**
- A7 core has **NO direct UART access to LoRa**
- Communication only via **controlled ICM channel**

### Layer 2: Application Manifests

Each application declares its capabilities in `app_manifest.json`:

**RTApp Manifest (M4 Core):**
```json
{
  "Capabilities": {
    "AllowedApplicationConnections": [ "HLAPP-COMPONENT-ID" ],
    "Uart": [ "ISU0" ],
    "Gpio": [ 8, 9, 0, 1, 2 ]
  }
}
```
- ✅ UART for LoRa communication
- ✅ GPIO for LEDs and control pins
- ✅ ICM to HLApp only
- ❌ NO network capabilities
- ❌ NO file system access

**HLApp Manifest (A7 Core):**
```json
{
  "Capabilities": {
    "AllowedApplicationConnections": [ "RTAPP-COMPONENT-ID" ],
    "AllowedConnections": [ "your-iothub.azure-devices.net" ],
    "DeviceAuthentication": "TENANT-ID"
  }
}
```
- ✅ Network to specific IoT Hub only
- ✅ ICM to RTApp only
- ✅ Certificate-based authentication
- ❌ NO UART access
- ❌ NO other network destinations

### Layer 3: Inter-Core Messaging Security

The ICM channel is the **ONLY** communication path between cores.

**Security Controls:**
1. **Message Type Validation:** Only telemetry and config messages allowed
2. **Size Validation:** Fixed-size telemetry structure enforced
3. **Content Validation:** Strict field range checking
4. **Rate Limiting:** Telemetry sent at controlled intervals

**ICM Message Types:**

| Type | Direction | Size | Contents |
|------|-----------|------|----------|
| Telemetry | RTApp → HLApp | Fixed (41 bytes) | Operational metrics only |
| Configuration | HLApp → RTApp | Variable | Control commands only |

**Validation Example (telemetry_validator.c):**
```c
bool Telemetry_Validate(const void *data, size_t size) {
    // Size check
    if (size != sizeof(TelemetryMessage)) {
        return false;  // Reject wrong-sized messages
    }
    
    // Range checks
    if (msg->rssi_count > 5) return false;
    if (msg->last_rssi[i] < -150 || msg->last_rssi[i] > 0) return false;
    if (msg->uptime_seconds > MAX_UPTIME) return false;
    
    return true;  // Passed all checks
}
```

### Layer 4: Application Logic Separation

**RTApp Design Principles:**
- **Local Processing Only:** All Reticulum packets processed in-core
- **No Payload Forwarding:** KISS frames, TNC operations stay local
- **Minimal Telemetry:** Only counters and RSSI statistics sent
- **Fail-Secure:** If ICM fails, radio continues working locally

**HLApp Design Principles:**
- **Cloud Management Only:** Device configuration and monitoring
- **Strict Validation:** Reject any suspicious ICM messages
- **Audit Logging:** Log all security events
- **Fail-Secure:** If cloud connection fails, RTApp continues working

### Layer 5: Azure IoT Hub Security

**Authentication:**
- **Certificate-Based:** No passwords or shared keys
- **Azure Sphere DPS:** Device Provisioning Service for auto-enrollment
- **Per-Device Identity:** Unique device certificates

**Communication:**
- **TLS 1.2+:** All cloud communication encrypted
- **Mutual TLS:** Both client and server authenticated
- **Message Signing:** Azure IoT Hub verifies message integrity

**Authorization:**
- **Component ID Restrictions:** Only authorized apps can communicate
- **Network Allowlist:** Only specific IoT Hub domain allowed
- **Tenant Binding:** Device tied to specific Azure tenant

## Security Verification

### Automated Checks

1. **Build-Time Validation:**
   - Manifest capability restrictions enforced by SDK
   - Component ID matching verified
   - Network allowlist validated

2. **Runtime Validation:**
   - ICM message size checks
   - Telemetry field range validation
   - Connection status monitoring

### Manual Verification Procedures

1. **ICM Traffic Analysis:**
   ```bash
   # Monitor RTApp logs for payload forwarding attempts
   azsphere device app show-log --component-id RTAPP-ID | grep "SECURITY"
   
   # Monitor HLApp logs for validation failures
   azsphere device app show-log --component-id HLAPP-ID | grep "SECURITY"
   ```

2. **Network Traffic Analysis:**
   ```bash
   # Verify RTApp has no network activity
   azsphere device network show-status
   
   # Verify HLApp only connects to IoT Hub
   azsphere device network show-proxy
   ```

3. **Capability Verification:**
   ```bash
   # List RTApp capabilities
   azsphere device app show-status --component-id RTAPP-ID
   
   # Verify no network capabilities listed
   ```

### Penetration Testing Scenarios

1. **Test: Attempt to send payload via ICM**
   - **Method:** Modify RTApp to send large message
   - **Expected:** HLApp rejects with size validation error
   - **Verify:** Check HLApp logs for rejection

2. **Test: Attempt network access from RTApp**
   - **Method:** Try to open socket in RTApp
   - **Expected:** System call fails (no network capability)
   - **Verify:** RTApp logs show permission denied

3. **Test: Attempt to connect to unauthorized host**
   - **Method:** Modify HLApp to connect to different host
   - **Expected:** Connection blocked by manifest
   - **Verify:** HLApp cannot establish connection

## Security Monitoring

### Key Indicators of Compromise

1. **Unexpected ICM Messages:**
   - Messages larger than expected telemetry size
   - Messages with invalid field values
   - Higher-than-normal message rate

2. **Network Anomalies:**
   - RTApp attempting network operations
   - HLApp connecting to unauthorized hosts
   - Unusual data volume to cloud

3. **Application Failures:**
   - Repeated validation errors
   - ICM communication failures
   - Azure IoT Hub authentication failures

### Logging Strategy

**RTApp Logs:**
```
Log_Debug("RTApp: Processed packet (len=%zu, rssi=%d)\n", length, rssi);
// DO NOT log packet contents (payload data)
```

**HLApp Logs:**
```
Log_Debug("SECURITY: Invalid telemetry size (expected=%zu, got=%zu)\n", ...);
Log_Debug("SECURITY ALERT: Suspiciously large message - possible payload!\n");
```

## Incident Response

### Response to Validation Failures

1. **Log the Event:**
   - Record timestamp, message size, source
   - Capture relevant context (not payload data)

2. **Drop the Message:**
   - DO NOT forward to cloud
   - DO NOT process further

3. **Alert Monitoring:**
   - Increment security event counter
   - Send alert telemetry to cloud

4. **Investigate:**
   - Review RTApp logs
   - Check for code modifications
   - Verify firmware integrity

### Response to Unauthorized Access

1. **Immediate Actions:**
   - Disable affected application
   - Revoke device certificates
   - Block device in IoT Hub

2. **Investigation:**
   - Review device logs
   - Check deployment history
   - Verify application images

3. **Remediation:**
   - Re-deploy known-good firmware
   - Rotate certificates
   - Update security policies

## Security Updates

### Update Process

1. **Azure Sphere OS Updates:**
   - Automatic security updates from Microsoft
   - Device reboots to apply updates
   - Automatic rollback on failure

2. **Application Updates:**
   - Deploy new image packages
   - Test in development mode first
   - Monitor for issues after deployment

### Security Patch Priority

| Severity | Response Time | Action |
|----------|---------------|--------|
| Critical | 24 hours | Emergency deployment |
| High | 1 week | Scheduled deployment |
| Medium | 1 month | Regular update cycle |
| Low | Next release | Include in next version |

## Compliance & Audit

### Security Documentation

1. **Architecture Diagrams:** This document and MT3620/README.md
2. **Threat Model:** Security threats and mitigations documented
3. **Code Comments:** Security-critical sections clearly marked
4. **Validation Logic:** Documented in telemetry_validator.c

### Audit Trail

- **Device Logs:** Application logs retained on device
- **Cloud Logs:** Azure IoT Hub logs all D2C/C2D messages
- **Security Events:** Tagged with "SECURITY" for easy filtering

## Best Practices

### For Developers

1. ✅ **DO** validate all ICM messages before processing
2. ✅ **DO** log security-relevant events
3. ✅ **DO** use fixed-size structures for telemetry
4. ✅ **DO** check return values from security functions
5. ❌ **DON'T** log payload contents
6. ❌ **DON'T** bypass validation checks
7. ❌ **DON'T** add network capabilities to RTApp
8. ❌ **DON'T** add UART access to HLApp

### For Operators

1. ✅ **DO** monitor security logs regularly
2. ✅ **DO** keep firmware updated
3. ✅ **DO** rotate certificates periodically
4. ✅ **DO** restrict network access to IoT Hub only
5. ❌ **DON'T** disable validation checks
6. ❌ **DON'T** use insecure network configurations
7. ❌ **DON'T** bypass Azure authentication
8. ❌ **DON'T** share device certificates

## References

- [Azure Sphere Security](https://docs.microsoft.com/azure-sphere/product-overview/security-features)
- [Azure IoT Hub Security](https://docs.microsoft.com/azure/iot-hub/iot-hub-security-x509-get-started)
- [MT3620 Hardware Security](https://www.mediatek.com/products/aiot/mt3620)

## Contact

For security issues or questions:
- Review this documentation first
- Check Azure Sphere security documentation
- Consult RNode Firmware community guidelines
