// Copyright (C) 2024, RNode Firmware Contributors
// Licensed under the GNU General Public License v3.0

#ifndef TELEMETRY_VALIDATOR_H
#define TELEMETRY_VALIDATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Validate that received data is operational telemetry, NOT Reticulum payload
// This is a CRITICAL SECURITY FUNCTION
// Returns true if data is valid telemetry, false otherwise
bool Telemetry_Validate(const void *data, size_t size);

#endif // TELEMETRY_VALIDATOR_H
