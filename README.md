# madronalib/mlvg stereo effect template

A minimal template for creating CLAP audio effect plugins using the madronalib and mlvg frameworks.

*This repository is not supported by Madrona Labs.*
Please do not email Madrona Labs about it.
File an issue here and I'll do what I can to help you out.

## Overview

This template provides a foundation for building stereo effect plugins (loopers, filters, delays, reverb, distortion, etc.) with a modern GUI. It's designed to be forked, renamed, and used as a base for your plugin.

CMake generates VST3, AUv2, and CLAP plugin files on macOS, Windows, and Linux.

TODO: testing, github actions

## Project Structure

```
mltemplate/
│
├── src/                          # plugin source code
│   ├── clap-stereo-effect-template.h
│   ├── clap-stereo-effect-template.cpp
│   ├── clap-stereo-effect-template-gui.h
│   └── clap-stereo-effect-template-gui.cpp
│
├── cmake/                        # CMake configuration
│   ├── CLAPPlugin.cmake          # main plugin creation
│   ├── CLAPTools.cmake           # CLAP development tools
│   └── README.md                 # tools documentation
│
├── external/                     # external dependencies
│   ├── madronalib/               # DSP framework
│   ├── mlvg/                     # GUI framework
│   ├── clap-validator/           # CLAP validation tool
│   ├── clap-info/                # CLAP inspection tool
│   └── clap-host/                # CLAP reference host
│
├── scripts/                      # build scripts
│
├── plugin-metadata.json          # single source of truth for plugin metadata
│
├── CMakeLists.txt                # build configuration
│
└── README.md                     # you are here :)
```

## Building/Installing

```bash
git clone http://github.com/username/mltemplate
cd mltemplate
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make -j # -jN uses N cores
make install
```

TODO: standard installers e.g. `.dmg`

## Quickstart

1. Fork this repo

2. Update metadata: Edit `plugin-metadata.json` with your plugin information

3. Define any persistent state in `ClapStereoEffectTemplate`'s `EffectState` in `processor.h` 

4. Define parameters in `ClapStereoEffectTemplate::buildParameterDescriptions()` in `processor.cpp`

5. Implement your effect processing in `ClapStereoEffectTemplate::processStereoEffect()` in `processor.cpp`

6. Create widgets in `ClapStereoEffectTemplateGUI::makeWidgets()` in `gui.cpp`

7. Layout widgets in `ClapStereoEffectTemplateGUI::layoutView()` in `gui.cpp`

8. Initialize visual resources in `ClapStereoEffectTemplateGUI::initializeResources()` in `gui.cpp`

9. Build: Use the standard build process described in the `Building/Installing` section above.

## Building Blocks

### DSP

#### Generators
- `ml::SineGen` - antialiased sine wave oscillator using Taylor series approximation
- `ml::SawGen` - antialiased sawtooth wave oscillator with polyBLEP
- `ml::PulseGen` - antialiased pulse wave oscillator with variable pulse width
- `ml::PhasorGen` - naive sawtooth generator (0-1 range) for wavetable control
- `ml::NoiseGen` - linear congruential generator white noise (spectrum depends on samplerate)
- `ml::TestSineGen` - slow but accurate sine generator for testing

#### Filters
- `ml::Lopass` - state variable filter
- `ml::Hipass` - state variable filter
- `ml::Bandpass` - state variable filter
- `ml::LoShelf` - bass boost/cut
- `ml::HiShelf` - treble boost/cut
- `ml::Bell` - midrange boost/cut
- `ml::OnePole` - simple one-pole filter
- `ml::DCBlocker` - one-pole, one-zero filter to remove DC offset
- `ml::ADSR` - attack-decay-sustain-release envelope generator
- `ml::IntegerDelay` - delay line with integer sample delay
- `ml::FractionalDelay` - delay line with fractional sample delay using allpass interpolation
- `ml::PitchbendableDelay` - click-free delay with crossfading for smooth pitch changes

#### Envelopes
- `ml::ADSR` - attack-decay-sustain-release envelope generator with gate input
- `ml::OneShotGen` - single-shot ramp generator (0-1) triggered by gate

#### Clocks & Timing
- `ml::TempoLock` - tempo-synchronized phasor for tempo-locked modulation
- `ml::TickGen` - clock tick generator for tempo-synchronized events
- `ml::ImpulseGen` - antialiased impulse generator for clean clock signals

#### Interpolation & Analysis
- `ml::LinearGlide` - linear interpolation between values with configurable glide time
- `ml::SampleAccurateLinearGlide` - sample-accurate linear interpolation for smooth parameter changes
- `ml::Interpolator1` - simple linear interpolation over DSPVector length
- `ml::Peak` - peak detector with exponential decay and hold time
- `ml::RMS` - root mean square detector for signal level measurement

### Widgets

#### Controls
- `ml::DialBasic` - circular control for continuous parameters (drag, mouse wheel, keyboard)
- `ml::TextButtonBasic` - clickable button with text label
- `ml::SVGButtonBasic` - clickable button with SVG icon
- `ml::ToggleButtonBasic` - on/off toggle button

#### Display
- `ml::TextLabelBasic` - displays text with configurable alignment, font, and styling
- `ml::SVGImage` - displays vector graphics from SVG files
- `ml::DrawableImageView` - displays rendered graphics from drawable images

#### Layout
- `ml::Panel` - container widget for grouping and styling other widgets
- `ml::Resizer` - handles window resizing with fixed aspect ratio constraints

## Fundamental classes/structs

### madronalib

- Core Types
  - `ml::Symbol` - interned string identifiers used as keys throughout the system
  - `ml::Path` - hierarchical addressing for tree structures (e.g., "widgets/controls/dial1")

- `ml::DSPVector`
  - for building fixed DSP graphs in a functional style
  - a time-ordered sequence of samples with length `const expr size_t kFloatsPerDSPVector = 64`
  - see `MLDSPOps.h` and `MLDSPMath.h`

- `ml::Event`
  - something that happens in a performance
  - note on, afterpressure, program change, etc
  - see `MLEvent.h`

- `ml::SignalProcessor`
  - converts incoming events to output signals (i.e. `DSPVector`s)
  - see `MLSignalProcessor.h`

- `ml::AudioContext`
  - where our signal processors meet the rest of the world
  - defines the sample rate, provides audio and event I/O
  - see `MLAudioContext.h`

- `ml::Message`
  - not for use in audio threads!
  - how `ml::Actor`s and `ml::MessageReceiver`s talk to each other

- `ml::Actor` and `ml::MessageReceiver` serve similar purposes
  - `ml::Actor`
    - asynchronous messaging
    - handles its own messages in its own queue
    - see `MLActor.h`
  - `ml::MessageReceiver`
    - synchronous messaging
    - designed for components that need ~immediate message processing (e.g. `ml::Widget`)
    - optional replies
      - Many senders do not expect a reply and send nullptr for replyPtr
      - The receiver must verify that replyPtr is non-null before attempting to reply
    - see `MLMessage.h`

- `ml::Tree`
  - the fundamental hierarchical data structure
  - recursive map from `ml::Path`s to values using `ml::Symbol` keys
  - provides depth-first iteration and path-based access
  - used by `ml::Collection`, `ml::PropertyTree`, and `ml::ParameterTree`
  - see `MLTree.h`

- `ml::PropertyTree`
  - stores widget properties and key-value data in a tree structure
  - type-safe access to properties with defaults
  - see `MLPropertyTree.h`

- `ml::ParameterTree`
  - stores parameter metadata (ranges, units, etc.)
  - handles conversion between `normalized` (0 to 1) and `real` (min,max) values
  - see `MLParameters.h`

- `ml::Collection`
  - template class for organizing objects in hierarchical trees
  - used extensively for managing widgets, parameters, and other hierarchical data
  - provides `CollectionRoot<T>` for creating root collections
  - see `MLCollection.h`

### mlvg

- `ml::Widget`
  - a drawable UI element stored in an `ml::View`
  - inherits from `ml::MessageReceiver`
  - has:
    - properties
      - determine the appearance of the widget
    - parameters
      - quantities (e.g. gain, frequency) in the `ml::SignalProcessor` controlled by the widget

- `ml::AppView`
  - inherits from `ml::Actor`
  - handles GUI events, sending messages to widgets and managing the view hierarchy
  - manages drawing resources and coordinates between platform and application

- `ml::View`
  - a special `ml::Widget` that contains other `ml::Widgets`
  - members:
    - `_widgets`, an `ml::Collection` of widgets
    - `_backgroundWidgets`, static widgets that should not overlap, since they can be drawn in any order 

- `ml::PlatformView`
  - platform-specific wrapper that draws the application into a native window
  - handles window management, scaling, and integration with the host system

- `ml::DrawContext`
  - provides drawing resources and coordinate system for widgets
  - contains native drawing context, fonts, images, and coordinate transformations

