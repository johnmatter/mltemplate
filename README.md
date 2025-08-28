# CLAP Saw Demo Plugin

A CLAP audio plugin template built on top of the `manzanita` framework from Madrona Labs.

## Requirements

- CMake 3.10 or higher
- C++17 compatible compiler
- Git (for submodules)

## Build Instructions

### 1. Clone and Initialize Submodules

```bash
git clone <repository-url>
cd mltemplate
git submodule update --init --recursive
```

### 2. Build the Plugin

```bash
mkdir build
cd build
cmake ..
make
```

### 3. Install the Plugin

The built plugin will be located in `build/clap/`. Copy it to your CLAP plugin directory:

**macOS:**
```bash
cp -r build/clap/clap-saw-demo.clap ~/Library/Audio/Plug-Ins/CLAP/
```

**Linux:**
```bash
cp build/clap/clap-saw-demo.clap ~/.clap/
```

**Windows:**
```bash
copy build\clap\clap-saw-demo.clap "%APPDATA%\CLAP\"
```

## License

This project uses the madronalib/mlvg framework which is distributed under the MIT license.
