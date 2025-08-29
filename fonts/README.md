# Custom Fonts Directory

Place your custom font files (OTF/TTF) in this directory to embed them in your CLAP plugin.

## Supported Formats
- OpenType Font (.otf)
- TrueType Font (.ttf)

## Usage
1. Add your font files to this directory
2. In your CMakeLists.txt, uncomment and modify:
   ```cmake
   add_custom_fonts(${PLUGIN_PROJECT_NAME} "fonts/")
   ```
3. In your GUI class, include the generated resources:
   ```cpp
   #include "../build/resources/your-plugin-name/resources.c"
   #include "font_loader.h"
   ```
4. Load your fonts in initializeResources():
   ```cpp
   LOAD_EMBEDDED_FONT(_resources, nvg, "my_font", my_font_otf);
   ```

## Font Naming
- Filename: `my_custom_font.otf`
- Generated resource: `resources::my_custom_font_otf`
- Generated size: `resources::my_custom_font_otf_size`
- Usage: `LOAD_EMBEDDED_FONT(_resources, nvg, "my_font", my_custom_font_otf);`

## Example
If you have `fonts/Roboto-Bold.ttf`:
```cpp
LOAD_EMBEDDED_FONT(_resources, nvg, "roboto_bold", Roboto_Bold_ttf);
```

## Documentation
See `docs/CUSTOM_FONTS.md` for complete documentation.
