#!/bin/bash
# Build script for RNode MT3620 Applications

set -e

echo "==================================="
echo "RNode MT3620 Build Script"
echo "==================================="

# Check if Azure Sphere SDK is installed
if ! command -v azsphere &> /dev/null; then
    echo "ERROR: Azure Sphere SDK not found"
    echo "Please install Azure Sphere SDK from:"
    echo "https://docs.microsoft.com/azure-sphere/install/overview"
    exit 1
fi

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found"
    echo "Please install CMake 3.10 or later"
    exit 1
fi

# Determine CMake generator
CMAKE_GENERATOR="Ninja"
if ! command -v ninja &> /dev/null; then
    echo "Warning: Ninja not found, using Unix Makefiles instead"
    CMAKE_GENERATOR="Unix Makefiles"
fi

# Build RTApp
echo ""
echo "Building RTApp (M4 Core)..."
cd RTApp
rm -rf build
cmake -B build -S . -G "$CMAKE_GENERATOR"
cmake --build build
echo "✓ RTApp build complete"

# Build HLApp
echo ""
echo "Building HLApp (A7 Core)..."
cd ../HLApp
rm -rf build
cmake -B build -S . -G "$CMAKE_GENERATOR"
cmake --build build
echo "✓ HLApp build complete"

echo ""
echo "==================================="
echo "Build Complete!"
echo "==================================="
echo ""
echo "Image packages created:"
echo "  RTApp: MT3620/RTApp/build/RNodeRTApp.imagepackage"
echo "  HLApp: MT3620/HLApp/build/RNodeHLApp.imagepackage"
echo ""
echo "To deploy:"
echo "  azsphere device sideload deploy --imagepackage RTApp/build/RNodeRTApp.imagepackage"
echo "  azsphere device sideload deploy --imagepackage HLApp/build/RNodeHLApp.imagepackage"
echo ""
