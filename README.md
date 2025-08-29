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
5. Build: Use the standard build process
5. Test: Use the integrated CLAP development tools

## CLAP Development Tools

This template includes integrated CLAP development tools for comprehensive testing and validation:

### Available Tools

- **clap-validator**: Automated validation and testing of CLAP plugins
- **clap-info**: Command-line tool to inspect CLAP plugins
- **clap-host**: Reference CLAP host implementation for standalone testing

### Building Tools

The tools are built by default when you run `make`. To disable them:

```bash
cmake -DBUILD_CLAP_TOOLS=OFF ..
```

### Using the Tools

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

- **clap-validator**: Rust and Cargo
- **clap-info**: CMake and C++ compiler
- **clap-host**: CMake and C++ compiler

If any requirements are missing, the corresponding tool will be skipped with a warning.

## Current Parameters

The template includes three basic parameters:
- Master Gain: Overall output level
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
