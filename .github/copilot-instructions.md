# RNode Firmware - GitHub Copilot Instructions

## Project Overview

This repository contains the open-source firmware for RNode devices - open, free, and unrestricted digital radio transceivers. RNode is a system that transforms various hardware devices into functional communication tools using LoRa technology.

**Key Technologies:**
- Arduino C/C++ for embedded systems
- Multiple platform support: ESP32, nRF52, AVR (ATmega)
- LoRa radio transceivers (SX1276, SX1278, SX1262, SX1268, SX1280)
- Arduino framework and libraries

**Important:** This repository functions as a stable reference for the default RNode Firmware and only receives bugfix and security updates. Further development happens in the [RNode Firmware Community Edition](https://github.com/liberatedsystems/RNode_Firmware_CE).

## Build System

### Prerequisites
- `arduino-cli` must be installed and configured
- Python 3 (for build scripts and tools)
- Make

### Setup Commands
```bash
# Initialize cores and dependencies for specific platforms:
make prep-avr      # For AVR-based boards (ATmega)
make prep-esp32    # For ESP32-based boards
make prep-nrf      # For nRF52-based boards
make prep-samd     # For SAMD-based boards
```

### Building Firmware
Use platform-specific make targets to build firmware:
```bash
# Examples:
make firmware-tbeam          # LilyGO T-Beam
make firmware-lora32_v21     # LilyGO LoRa32 v2.1
make firmware-heltec32_v3    # Heltec LoRa32 v3
make firmware-rak4631        # RAK4631 nRF52 board
```

### Release Builds
```bash
make release          # Build all release variants
make release-hashes   # Generate release hash checksums
```

## Project Structure

- **Root directory:** Arduino .ino firmware source and header files
- **Boards.h:** Hardware board definitions and pin mappings
- **Config.h:** Firmware configuration constants
- **Modem.h, sx127x.cpp, sx126x.cpp, sx128x.cpp:** LoRa modem drivers
- **Release/:** Build output directory for firmware releases
- **Console/:** Bootstrap console web interface
- **Documentation/:** Project documentation and images
- **Makefile:** Build system configuration
- **arduino-cli.yaml:** Arduino CLI configuration

## Coding Standards

### Language Standards
- **C++ for Arduino:** Follow Arduino coding style conventions
- Use clear, descriptive variable and function names
- Keep functions focused and modular

### Code Organization
- Hardware-specific code should be conditionally compiled using preprocessor directives
- Board configurations are defined in `Boards.h` using `BOARD_MODEL` constants
- Modem types are specified via `MODEM` preprocessor definitions

### Comments
- Use clear comments for hardware-specific sections
- Document pin assignments and hardware configurations
- Explain non-obvious logic, especially for radio/modem operations

### Preprocessor Usage
- Board models use hexadecimal constants (e.g., `0x33` for T-Beam)
- Modem types: `0x01` (SX127x), `0x03` (SX126x), `0x04` (SX1280)
- Use `#ifdef`/`#ifndef` for conditional compilation

## Testing

**Note:** This repository does not have automated unit tests. Changes should be:
1. Compiled successfully for the target board
2. Tested on physical hardware when possible
3. Verified to not break existing functionality

## Dependencies

### Arduino Libraries (installed via arduino-cli)
- Adafruit SSD1306
- Adafruit SH110X
- Adafruit ST7735 and ST7789 Library
- Adafruit NeoPixel
- XPowersLib
- Crypto
- GxEPD2 (for nRF52 boards)

### External Tools
- `esptool.py` (included in Release/esptool/)
- `adafruit-nrfutil` (for nRF52 packaging)
- `rnodeconf` (RNode configuration utility)

## Important Constraints

### License Compliance
- This project is licensed under **GNU General Public License v3.0**
- All code contributions must be compatible with GPLv3
- The LoRa modem drivers (sx127x.cpp, sx126x.cpp, sx128x.cpp) are under MIT License (Copyright Sandeep Mistry, Mark Qvist and Jacob Eva)
- Maintain proper copyright notices and license headers

### Security Considerations
- This is embedded firmware for radio devices
- Be cautious with memory management (limited RAM on embedded devices)
- Avoid buffer overflows and memory leaks
- Validate input data, especially from serial communications

### Hardware Limitations
- **Memory constraints:** AVR boards have very limited RAM (8-16KB)
- **Flash constraints:** Firmware must fit in available flash memory
- **Real-time requirements:** Radio timing is critical for proper operation

## Common Tasks

### Adding Support for a New Board
1. Add board definition to `Boards.h`
2. Define pin mappings for the specific hardware
3. Add compile target in `Makefile`
4. Test thoroughly on physical hardware

### Updating Modem Drivers
- Modem code is in `sx127x.cpp`, `sx126x.cpp`, `sx128x.cpp`
- Changes must maintain compatibility with existing configurations
- Test across multiple board types if possible

### Modifying Build Configuration
- Update `Makefile` for new targets or build options
- Ensure `arduino-cli.yaml` has necessary board manager URLs
- Update build properties as needed for different boards

## File Naming Conventions
- Use descriptive names for configuration and feature files
- Header guards should match filename (uppercase with underscores)
- Keep firmware releases in `Release/` directory

## Version Control
- This repository receives **only bugfixes and security updates**
- New features should go to the Community Edition repository
- Keep commits focused and well-described
- Reference issue numbers in commit messages when applicable

## Support Resources
- Main documentation: [unsigned.io](https://unsigned.io)
- Installation guide: [RNode firmware installation](https://unsigned.io/guides/2022_01_25_installing-rnode-firmware-on-supported-devices.html)
- Community vendor: [Liberated Embedded Systems](https://store.liberatedsystems.co.uk/)
