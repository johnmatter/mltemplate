# CLAP Stereo Effect Template

A minimal, clean template for creating stereo effect plugins using madronalib and mlvg.

## Overview

This template provides a foundation for creating stereo effect plugins with:
- Stereo input/output processing
- Parameter system with automatic GUI binding
- Modern GUI framework
- CLAP plugin format support

## Building

```bash
mkdir build
cd build
cmake ..
make
```

The plugin will be built as `clap-stereo-effect-template.clap` in the `build/clap/` directory.

## Project Structure

```
src/
├── clap-stereo-effect-template.h          # Main plugin header
├── clap-stereo-effect-template.cpp        # Main plugin implementation
├── clap-stereo-effect-template-gui.h      # GUI header
├── clap-stereo-effect-template-gui.cpp    # GUI implementation
└── clap-stereo-effect-template-entry.cpp  # Plugin entry point
```

## Creating Your Effect

### 1. Add Effect State
In `clap-stereo-effect-template.h`, add your effect-specific state to the `EffectState` struct:

```cpp
struct EffectState {
  // Add your effect-specific state here
  float leftGain = 1.0f;
  float rightGain = 1.0f;
};
```

### 2. Add Parameters
In `buildParameterDescriptions()`, add your effect parameters:

```cpp
params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
  {"name", "delay_time"},
  {"range", {0.001f, 2.0f}},
  {"plaindefault", 0.5f},
  {"units", "s"}
}));
```

### 3. Implement Effect Processing
In `processStereoEffect()`, replace the template code with your effect:

```cpp
void ClapStereoEffectTemplate::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Get parameters
  float delayTime = this->getRealFloatParam("delay_time");
  
  // Process your effect
  leftChannel = effectState.delayBuffer.process(leftChannel, delayTime);
  rightChannel = effectState.delayBuffer.process(rightChannel, delayTime);
}
```

### 4. Add GUI Widgets
In `makeWidgets()`, add controls for your parameters:

```cpp
_view->_widgets.add_unique<DialBasic>("delay_time", ml::WithValues{
  {"bounds", {0, 1.5, 2.5, 2.5}},
  {"log", true},
  {"param", "delay_time"}
});
```

## Dependencies

- **madronalib**: DSP framework and plugin infrastructure
- **mlvg**: Graphics and GUI framework
- **CLAP**: Plugin format specification

## License

This template is provided as-is for educational and development purposes.
