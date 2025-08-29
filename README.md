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
5. Test: TODO: tests 

## Current Parameters

The template includes three basic parameters:
- Master Gain: Overall output level
- Left Gain: Left channel gain
- Right Gain: Right channel gain

## Dependencies

- madronalib: DSP and plugin framework
- mlvg: Graphics and GUI framework
- CLAP: Plugin format specification
