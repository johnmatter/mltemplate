#!/bin/bash
# Build VST3 Plugin Test Host separately
# This script downloads and builds the VST3 SDK with hosting examples enabled

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
VST3_BUILD_DIR="${PROJECT_ROOT}/build/vst3-testhost"

echo "Building VST3 Plugin Test Host..."
echo "Build directory: ${VST3_BUILD_DIR}"

# Create build directory
mkdir -p "${VST3_BUILD_DIR}"
cd "${VST3_BUILD_DIR}"

# Download complete VST3 SDK with all submodules for building hosting examples
echo "Downloading complete VST3 SDK..."
if [ ! -d "vst3sdk" ]; then
    git clone --recursive --branch v3.7.6_build_18 https://github.com/steinbergmedia/vst3sdk.git
else
    echo "VST3 SDK already exists, updating submodules..."
    cd vst3sdk
    git submodule update --init --recursive
    cd ..
fi
VST3_SOURCE="${VST3_BUILD_DIR}/vst3sdk"

# Configure with hosting examples enabled
echo "Configuring VST3 SDK with hosting examples..."

# Check if VST3 SDK has the required structure and files
if [ ! -f "${VST3_SOURCE}/CMakeLists.txt" ]; then
    echo "Error: VST3 SDK CMakeLists.txt not found at ${VST3_SOURCE}"
    exit 1
fi

if [ ! -d "${VST3_SOURCE}/cmake" ]; then
    echo "Error: VST3 SDK cmake modules not found at ${VST3_SOURCE}/cmake"
    echo "The VST3 SDK might be incomplete. Try deleting ${VST3_SOURCE} and running again."
    exit 1
fi

# Run CMake configuration from the VST3 source directory as root project
cd "${VST3_SOURCE}"
cmake -S . -B "${VST3_BUILD_DIR}" \
    -DSMTG_ADD_VST3_HOSTING_SAMPLES=ON \
    -DSMTG_ADD_VST3_PLUGINS_SAMPLES=OFF \
    -DSMTG_ADD_VSTGUI=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# Switch back to build directory
cd "${VST3_BUILD_DIR}"

# Build the hosting examples
echo "Building VST3 hosting examples..."
cmake --build . --config Release --target editorhost validator

# Check if executables were built
echo ""
echo "Checking for built executables..."

EDITORHOST_PATH=$(find . -name "editorhost" -type f 2>/dev/null | head -1)
if [ -n "$EDITORHOST_PATH" ]; then
    echo "✓ VST3 editorhost built successfully: ${VST3_BUILD_DIR}/${EDITORHOST_PATH}"
else
    echo "⚠ editorhost executable not found"
fi

VALIDATOR_PATH=$(find . -name "validator" -type f 2>/dev/null | head -1)
if [ -n "$VALIDATOR_PATH" ]; then
    echo "✓ VST3 validator built successfully: ${VST3_BUILD_DIR}/${VALIDATOR_PATH}"
else
    echo "⚠ validator executable not found"
fi

# List all VST3 hosting related executables found
echo ""
echo "All VST3 hosting executables found:"
find . -name "*host*" -type f 2>/dev/null || true
find . -name "*validator*" -type f 2>/dev/null || true

echo ""
echo "VST3 Test Hosts Usage:"
echo "----------------------"
echo "editorhost: VST3 host with GUI editor support"
echo "validator: VST3 plugin validator for compliance testing"
echo ""
echo "To test your plugin:"
if [ -n "$EDITORHOST_PATH" ]; then
    echo "./${EDITORHOST_PATH} --help"
fi
if [ -n "$VALIDATOR_PATH" ]; then
    echo "./${VALIDATOR_PATH} --help"
fi
echo ""
echo "Your VST3 plugin is located at: ${PROJECT_ROOT}/build/bin/vst3/TapeHack.vst3"
