# CLAP Development Tools

This directory contains CMake modules for building and using CLAP development tools.

## Available Tools

### clap-validator
- **Purpose**: Automated validation and testing of CLAP plugins
- **Language**: Rust
- **Source**: [free-audio/clap-validator](https://github.com/free-audio/clap-validator)
- **Usage**: Validates plugins for common bugs and incorrect behavior

### clap-info
- **Purpose**: Command-line tool to inspect CLAP plugins
- **Language**: C++
- **Source**: [free-audio/clap-info](https://github.com/free-audio/clap-info)
- **Usage**: Scans and displays plugin information, parameters, ports, etc.

### clap-host
- **Purpose**: Reference CLAP host implementation
- **Language**: C++
- **Source**: [free-audio/clap-host](https://github.com/free-audio/clap-host)
- **Usage**: Standalone host for testing plugins

## CMake Modules

### CLAPTools.cmake
Master module that includes all tools and provides convenient targets.

### CLAPValidator.cmake
Builds clap-validator using Cargo.

### CLAPInfo.cmake
Builds clap-info using CMake.

### CLAPHost.cmake
Builds clap-host using CMake.

## Usage

### Building Tools
```bash
# Build all tools
make clap-tools-build

# Build individual tools
make clap-validator-build
make clap-info-build
make clap-host-build
```

### Running Tests
```bash
# Run complete test suite
make clap-test-suite

# Run individual tests
make clap-validate    # Validate plugins
make clap-info        # Get plugin information
make clap-host        # Test in standalone host
```

### Disabling Tools
To disable CLAP tools build:
```bash
cmake -DBUILD_CLAP_TOOLS=OFF ..
```

## Requirements

- **clap-validator**: Rust and Cargo
- **clap-info**: CMake and C++ compiler
- **clap-host**: CMake and C++ compiler

## Output Locations

Tools are built in the `build/external/` directory:
- `build/external/clap-validator/release/clap-validator`
- `build/external/clap-info/clap-info`
- `build/external/clap-host/host/clap-host`

## Troubleshooting

### Submodule Issues
If you encounter build failures related to missing CMakeLists.txt files, the CLAP tools may need their submodules initialized:

```bash
# Initialize submodules for CLAP tools
git submodule update --init --recursive external/clap-info
git submodule update --init --recursive external/clap-host
```

### Missing Dependencies
- **Rust/Cargo**: Required for clap-validator. Install from https://rustup.rs/
- **Qt**: Required for clap-host. Install via your system package manager
- **CMake**: Required for clap-info and clap-host
