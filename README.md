# CLAP Stereo Effect Template

A minimal template for creating CLAP audio effect plugins using the madronalib and mlvg frameworks.

## Overview

This template provides a foundation for building stereo effect plugins (loopers, filters, delays, reverb, distortion, etc.) with a modern GUI. It's designed to be forked and customized.

## Project Structure

```
mltemplate/
├── src/                          # plugin source code
│   ├── clap-stereo-effect-template.h
│   ├── clap-stereo-effect-template.cpp
│   ├── clap-stereo-effect-template-gui.h
│   └── clap-stereo-effect-template-gui.cpp
├── cmake/                        # CMake configuration
│   ├── CLAPPlugin.cmake          # main plugin creation
│   ├── CLAPTools.cmake           # CLAP development tools
│   └── README.md                 # tools documentation
├── external/                     # external dependencies
│   ├── madronalib/               # DSP framework
│   ├── mlvg/                     # GUI framework
│   ├── clap-validator/           # CLAP validation tool
│   ├── clap-info/                # CLAP inspection tool
│   └── clap-host/                # CLAP reference host
├── scripts/                      # build scripts
├── plugin-metadata.json          # single source of truth for plugin metadata
├── CMakeLists.txt                # build configuration
└── README.md                     # you are here :)
```

## Building

```bash
mkdir build
cd build
cmake ..
make -j # -jN uses N cores
```

The build system automatically:
- Parses metadata from `plugin-metadata.json`
- Generates the CLAP entry point in `src/clap-stereo-effect-template-entry.cpp`
- Creates the plugin bundle with proper metadata
- Builds CLAP development tools (optional, enabled by default)

## Installing

```bash
make install
```

The plugin will be installed to the appropriate CLAP directory for your platform:
- macOS: `~/Library/Audio/Plug-Ins/CLAP/`
- Linux: `~/.clap/`
- Windows: `%APPDATA%/CLAP/`

TODO: standard installers e.g. `.dmg`

## Plugin Metadata

All plugin metadata is centralized in `plugin-metadata.json` including:

- Plugin name and creator
- Version information
- Bundle identifiers
- CLAP properties

## Creating Your Effect

1. Fork this template
2. Update metadata: Edit `plugin-metadata.json` with your plugin information
3. Customize parameters: Add your parameters to the processor and GUI
4. Implement your effect: Replace the basic gain effect in `processStereoEffect()` with your DSP
5. Add custom fonts (optional): Use the font embedding system for custom typography
6. Build: Use the standard build process
7. Test: Use the integrated CLAP development tools

## CLAP Development Tools

This template includes integrated CLAP development tools for comprehensive testing and validation:

### Available Tools

- clap-validator: Automated validation and testing of CLAP plugins
- clap-info: Command-line tool to inspect CLAP plugins
- clap-host: Reference CLAP host implementation for standalone testing

### Building Tools

The tools are built by default when you run `make`. To disable them:

```bash
cmake -DBUILD_CLAP_TOOLS=OFF ..
```

### Using the Tools

```bash
# Validate your CLAP plugin
./build/external/clap-validator/release/clap-validator ./build/clap/TapeHack.clap

# Get detailed plugin information
./build/external/clap-info/clap-info ./build/clap/TapeHack.clap

# Test in standalone host
./build/external/clap-host/host/clap-host ./build/clap/TapeHack.clap
```

## Plugin Validation

This template includes comprehensive validation tools for all supported plugin formats:

### Validation Tools

- **CLAP**: clap-validator (official CLAP validation tool)
- **VST3**: pluginval (comprehensive VST3 testing)
- **AUv2**: pluginval (comprehensive AUv2 testing)

### Building Validation Tools

Validation tools are built by default. To customize:

```bash
# Build everything including validation tools
cmake -DBUILD_CLAP_TOOLS=ON -DBUILD_CLAP_WRAPPERS=ON -DBUILD_PLUGINVAL=ON ..

# Build only CLAP tools
cmake -DBUILD_CLAP_TOOLS=ON -DBUILD_CLAP_WRAPPERS=OFF -DBUILD_PLUGINVAL=OFF ..

# Build only wrappers and validation
cmake -DBUILD_CLAP_TOOLS=OFF -DBUILD_CLAP_WRAPPERS=ON -DBUILD_PLUGINVAL=ON ..
```

### Automated Validation

Use the validation script for comprehensive testing:

```bash
# Validate all plugin formats
./scripts/validate-plugins.sh

# Validate with custom settings
./scripts/validate-plugins.sh \
  --build-dir build \
  --plugin-name TapeHack \
  --strictness-level 7

# Validate specific formats
./scripts/validate-plugins.sh --help
```

### CI/CD Integration

The template includes GitHub Actions workflows for automated validation:

- **validate-plugins.yml**: Comprehensive validation on multiple platforms
- **validate-clap-only.yml**: CLAP-only validation for quick feedback
- **validate-wrappers-only.yml**: Wrapper validation for cross-platform testing

### Manual Validation Commands

```bash
# CLAP validation
./build/external/clap-validator/release/clap-validator ./build/clap/TapeHack.clap

# VST3 validation (strictness level 5 = host compatible)
./build/external/pluginval/pluginval --strictness-level 5 ./build/bin/vst3/TapeHack.vst3

# AUv2 validation (strictness level 5 = host compatible)
./build/external/pluginval/pluginval --strictness-level 5 ./build/bin/au2/TapeHack.component
```

### Validation Levels

pluginval strictness levels:
- **1-4**: Basic crash testing and call coverage
- **5**: Host compatibility (recommended for production)
- **6-10**: Extended testing including parameter fuzzing and state restoration

```bash
# Build all tools
make clap-tools-build

# Run complete test suite
make clap-test-suite

# Individual operations
make clap-validate        # Validate plugin compliance
make clap-info           # Get plugin information
make clap-host           # Test in standalone host
```

### Requirements

- clap-validator: Rust and Cargo
- clap-info: CMake and C++ compiler
- clap-host: CMake and C++ compiler

If any requirements are missing, the corresponding tool will be skipped with a warning.

## Custom Fonts

This template includes a clean system for embedding custom fonts in your CLAP plugin:

### Quick Start

1. Create a `fonts/` directory and add your OTF/TTF files
2. Uncomment in `CMakeLists.txt`:
   ```cmake
   add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")
   ```
3. Include the font loader in your GUI:
   ```cpp
   #include "font_loader.h"
   ```
4. Load fonts in `initializeResources()`:
   ```cpp
   LOAD_EMBEDDED_FONT(_resources, nvg, "my_font", MyFont_otf);
   ```

### Documentation

See `docs/CUSTOM_FONTS.md` for complete documentation including:
- Font naming conventions
- Advanced usage examples
- Troubleshooting guide
- Best practices

## Current Parameters

The template includes three basic parameters:
- Main Gain: Overall output level
- Left Gain: Left channel gain
- Right Gain: Right channel gain

## Dependencies

- madronalib: DSP and plugin framework
- mlvg: Graphics and GUI framework
- CLAP: Plugin format specification

### Development Dependencies

The following are optional and only needed for CLAP development tools:

- Rust and Cargo: For clap-validator
- CMake and C++ compiler: For clap-info and clap-host
