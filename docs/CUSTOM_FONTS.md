# Custom Font Embedding in CLAP Plugins

This document explains how to embed custom fonts in your CLAP plugin using MLVG's font embedding system.

## Overview

MLVG provides a clean way to embed custom fonts (OTF/TTF) directly into your CLAP plugin binary. This ensures your plugin always has the fonts it needs, regardless of the host system.

## Quick Start

### 1. Create Fonts Directory

```bash
mkdir fonts
```

### 2. Add Your Font Files

Place your OTF/TTF files in the `fonts/` directory:

```
fonts/
├── Roboto-Regular.ttf
├── Roboto-Bold.ttf
└── MyCustomFont.otf
```

### 3. Update CMakeLists.txt

Uncomment and modify the font embedding lines:

```cmake
# Optional: Add custom fonts
create_fonts_directory()  # Creates fonts/ directory with instructions
add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")
```

### 4. Update Your GUI Class

Include the generated resources:

```cpp
// In your GUI .cpp file
#include "../build/resources/clap-stereo-effect-template/resources.c"
#include "../build/font_resources/clap-stereo-effect-template/resources.c"
```

### 5. Load Fonts in initializeResources()

```cpp
void MyPluginGUI::initializeResources(NativeDrawContext* nvg) override {
  // Load embedded fonts directly
  _resources.fonts["roboto_regular"] = std::make_unique<ml::FontResource>(
    nvg, "roboto_regular", 
    resources::Roboto_Regular_ttf, 
    resources::Roboto_Regular_ttf_size, 0);
  
  _resources.fonts["roboto_bold"] = std::make_unique<ml::FontResource>(
    nvg, "roboto_bold", 
    resources::Roboto_Bold_ttf, 
    resources::Roboto_Bold_ttf_size, 0);
  
  _resources.fonts["my_custom"] = std::make_unique<ml::FontResource>(
    nvg, "my_custom", 
    resources::MyCustomFont_otf, 
    resources::MyCustomFont_otf_size, 0);
  
  // Set up your GUI...
}
```

### 6. Use Fonts in Your Widgets

```cpp
_view->_widgets.add_unique<TextLabelBasic>("title", ml::WithValues{
  {"bounds", {0, 0.2, 8, 0.5}},
  {"text", "My Plugin"},
  {"font", "roboto_bold"},  // Use your custom font
  {"text_size", 0.3f},
  {"h_align", "center"},
  {"v_align", "middle"}
});
```

## Font Naming Convention

The `create_resources()` function converts filenames to C-compatible names:

| Filename | Generated Resource | Generated Size | Usage |
|----------|-------------------|----------------|-------|
| `Roboto-Regular.ttf` | `resources::Roboto_Regular_ttf` | `resources::Roboto_Regular_ttf_size` | `_resources.fonts["roboto_regular"] = std::make_unique<ml::FontResource>(nvg, "roboto_regular", resources::Roboto_Regular_ttf, resources::Roboto_Regular_ttf_size, 0);` |
| `MyCustomFont.otf` | `resources::MyCustomFont_otf` | `resources::MyCustomFont_otf_size` | `_resources.fonts["my_custom"] = std::make_unique<ml::FontResource>(nvg, "my_custom", resources::MyCustomFont_otf, resources::MyCustomFont_otf_size, 0);` |

- Spaces, hyphens, and dots are replaced with underscores
- Case is preserved
- File extension is included in the resource name

## Generated Files

When you run `add_custom_fonts()`, CMake generates:

```
build/font_resources/your-plugin-name/
├── resources.c          # Main include file
├── Roboto_Regular_ttf.c # Font data array
├── Roboto_Bold_ttf.c    # Font data array
└── MyCustomFont_otf.c   # Font data array
```

## Font Loading Pattern

Fonts are loaded directly in the `initializeResources()` method:

```cpp
_resources.fonts["font_name"] = std::make_unique<ml::FontResource>(
  nvg, "font_name", 
  resources::GeneratedFontName_ttf, 
  resources::GeneratedFontName_ttf_size, 0);
```

Parameters:
- `nvg`: The `NativeDrawContext` from `initializeResources`
- `font_name`: The name you want to use for the font (e.g., "roboto_bold")
- `GeneratedFontName_ttf`: The generated resource name (e.g., `Roboto_Bold_ttf`)
- `GeneratedFontName_ttf_size`: The generated size variable

## Complete Example

### CMakeLists.txt
```cmake
# Include font embedding utilities
include(FontEmbedding)

# Create the CLAP plugin
create_clap_plugin(${PLUGIN_PROJECT_NAME})

# Add custom fonts
add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")
```

### GUI Header (my-plugin-gui.h)
```cpp
#pragma once
#include "CLAPExport.h"

class MyPluginGUI : public ml::CLAPAppView<MyPlugin> {
public:
  MyPluginGUI(MyPlugin* processor);
  void makeWidgets() override;
  void layoutView(ml::DrawContext dc) override;
  void initializeResources(NativeDrawContext* nvg) override;
};
```

### GUI Implementation (my-plugin-gui.cpp)
```cpp
#include "my-plugin-gui.h"
#include "../build/resources/my-plugin/resources.c"
#include "../build/font_resources/my-plugin/resources.c"

MyPluginGUI::MyPluginGUI(MyPlugin* processor)
  : CLAPAppView("MyPlugin", processor) {
  // Set up your GUI...
}

void MyPluginGUI::initializeResources(NativeDrawContext* nvg) override {
  // Load embedded fonts directly
  _resources.fonts["roboto_regular"] = std::make_unique<ml::FontResource>(
    nvg, "roboto_regular", 
    resources::Roboto_Regular_ttf, 
    resources::Roboto_Regular_ttf_size, 0);
  
  _resources.fonts["roboto_bold"] = std::make_unique<ml::FontResource>(
    nvg, "roboto_bold", 
    resources::Roboto_Bold_ttf, 
    resources::Roboto_Bold_ttf_size, 0);
  
  // Set up visual style
  _drawingProperties.setProperty("mark", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 }));
  _drawingProperties.setProperty("background", ml::colorToMatrix({ 0.8, 0.8, 0.8, 1.0 }));
}

void MyPluginGUI::makeWidgets() {
  _view->_widgets.add_unique<TextLabelBasic>("title", ml::WithValues{
    {"bounds", {0, 0.2, 8, 0.5}},
    {"text", "My Plugin"},
    {"font", "roboto_bold"},  // Use custom font
    {"text_size", 0.3f},
    {"h_align", "center"},
    {"v_align", "middle"}
  });
  
  // Add more widgets...
}
```
