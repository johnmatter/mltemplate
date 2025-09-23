#include "ChordGenerator-gui.h"
#include "ChordGenerator.h"
#include <cstdlib>
#include <string>

// Include embedded font resources
#include "../build/resources/ChordGenerator/resources.c"
#include "../build/font_resources/ChordGenerator/resources.c"

// Constructor - plugin-specific implementation
ChordGeneratorGUI::ChordGeneratorGUI(ChordGenerator* processor)
  : CLAPAppView("ChordGenerator", processor) {

  // Set up grid system for fixed aspect ratio
  setGridSizeDefault(kDefaultGridSize);
  setGridSizeLimits(kMinGridSize, kMaxGridSize);
  setFixedAspectRatio({kGridUnitsX, kGridUnitsY});
}

// Pure virtual override from CLAPAppView
void ChordGeneratorGUI::makeWidgets() {

  _view->_widgets.add_unique<TextLabelBasic>("title", ml::WithValues{
    {"bounds", {0.02*kGridUnitsX, 0.0, 0.8*kGridUnitsX, 1.0}},
    {"text", "ChordGenerator"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("title_text_size")},
    {"h_align", "left"},
    {"v_align", "middle"},
    {"text_color", _drawingProperties.getMatrixProperty("text_color")}
  });

  // Harmonics parameter - chord selection
  _view->_widgets.add_unique<DialBasic>("harmonics", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("harmonics_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "harmonics"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("harmonics_label", ml::WithValues{
    {"text", "chord"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", _drawingProperties.getMatrixProperty("text_color")},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // Inversion parameter - chord inversion/voicing
  _view->_widgets.add_unique<DialBasic>("inversion", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("inversion_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "inversion"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("inversion_label", ml::WithValues{
    {"text", "invert"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", _drawingProperties.getMatrixProperty("text_color")},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // level parameter - overall output level
  _view->_widgets.add_unique<DialBasic>("level", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("level_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "level"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("level_label", ml::WithValues{
    {"text", "level"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", _drawingProperties.getMatrixProperty("text_color")},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // Detune parameter - oscillator detuning in cents
  _view->_widgets.add_unique<DialBasic>("detune", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("detune_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "detune"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("detune_label", ml::WithValues{
    {"text", "detune"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", _drawingProperties.getMatrixProperty("text_color")},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // horizontal separator line
  _view->_widgets.add_unique<LineWidget>("separator_line", ml::WithValues{
    {"bounds", {0.1, 0.4, 8.8, 1.0}},  // x, y, width, height
    {"color", _drawingProperties.getMatrixProperty("text_color")},  // gray color
    {"thickness", 4.0f},  // 2 pixel thick line
    {"opacity", 0.8f}     // 80% opacity
  });

  // Add oscilloscope widget in the lower section
  _view->_widgets.add_unique<OscilloscopeWidget>("oscilloscope", ml::WithValues{
    {"bounds", {0.5f, 3.0f, 8.0f, 1.8f}},  // x, y, width, height - positioned below dials
    {"visible", true},  // CRITICAL: Must be explicitly set to visible
    {"timebase_scale", 1.0f},
    {"amplitude_scale", 1.0f},
    {"trigger_level", 0.0f},
    {"trigger_channel", 0},
    {"trigger_enable", true},
    {"signal_name", "scope_output"}  // Connect to the signal published by processor
  });

  // Add resize widget to bottom right corner
  _view->_backgroundWidgets.add_unique<Resizer>("resizer", ml::WithValues{
    {"fix_ratio", static_cast<float>(kGridUnitsX)/static_cast<float>(kGridUnitsY)},  // Use grid constants for aspect ratio
    {"z", -2},                  // Stay on top of other widgets
    {"fixed_size", true},
    {"fixed_bounds", {-16, -16, 16, 16}},  // 16x16 pixel resize handle
    {"anchor", {1, 1}}          // Anchor to bottom right corner {1, 1}
  });
}

// Override from AppView - called when GUI needs to update widget positions
void ChordGeneratorGUI::layoutView(ml::DrawContext dc) {

  // Helper lambda - plugin-specific utility for positioning dial labels
  auto positionLabelUnderDial = [&](ml::Path dialName, ml::Path labelName) {
    if (!_view->_widgets[dialName] || !_view->_widgets[labelName]) {
      return; // Widgets not created yet or we can't find them
    }

    ml::Rect dialRect = _view->_widgets[dialName]->getRectProperty("bounds");

    // Position label with same width and horizontal alignment as dial
    float yGap = -0.3f;
    float labelY = dialRect.top() + yGap;

    // Get current label bounds and update position with dial's width
    ml::Rect currentBounds = _view->_widgets[labelName]->getRectProperty("bounds");
    ml::Rect newBounds(dialRect.left(), labelY, dialRect.width(), currentBounds.height());

    _view->_widgets[labelName]->setRectProperty("bounds", newBounds);
  };

  // Position ChordGenerator dials
  positionLabelUnderDial("harmonics", "harmonics_label");
  positionLabelUnderDial("inversion", "inversion_label");
  positionLabelUnderDial("level", "level_label");
  positionLabelUnderDial("detune", "detune_label");

}

// Pure virtual override from CLAPAppView - must implement to set up fonts, colors, and layout
void ChordGeneratorGUI::initializeResources(NativeDrawContext* nvg) {
  if (!nvg) return;

  // Set up visual style for this plugin
  _drawingProperties.setProperty("mark", ml::colorToMatrix({ 0.1, 0.1, 0.1, 1.0 }));
  _drawingProperties.setProperty("mark_bright", ml::colorToMatrix({ 0.1, 0.1, 0.1, 1.0 }));
  _drawingProperties.setProperty("background", ml::colorToMatrix(nvgHSL(99.0 / 360, 0.5f, 0.9f)));
  _drawingProperties.setProperty("text_color", ml::colorToMatrix({ 0.1, 0.1, 0.1, 1.0 }));
  _drawingProperties.setProperty("line_color", ml::colorToMatrix({ 0.1, 0.1, 0.1, 1.0 }));
  _drawingProperties.setProperty("common_stroke_width", 1 / 32.f);

  // Centralized typography
  _drawingProperties.setProperty("title_text_size", 0.5f);
  _drawingProperties.setProperty("label_text_size", 0.3f);
  _drawingProperties.setProperty("dial_text_size", 0.5f);

  // Dial properties
  _drawingProperties.setProperty("dial_size", 0.7f);      // Visual size of the dial knob
  _drawingProperties.setProperty("dial_bounds", 1.6f);   // Bounds size for positioning

  // Single row for all dials
  _drawingProperties.setProperty("dial_row_y", 1.4f);

  // Column positions for chord dials in one row
  float dialBounds = _drawingProperties.getFloatProperty("dial_bounds");
  float totalWidth = kGridUnitsX;
  float spacing = (totalWidth - 4 * dialBounds) / 5.0f; // Equal spacing between dials and edges

  _drawingProperties.setProperty("harmonics_dial_x", spacing * 1 + dialBounds * 0);
  _drawingProperties.setProperty("inversion_dial_x", spacing * 2 + dialBounds * 1);
  _drawingProperties.setProperty("level_dial_x", spacing * 3 + dialBounds * 2);
  _drawingProperties.setProperty("detune_dial_x", spacing * 4 + dialBounds * 3);

  // Load embedded fonts (essential for text to work properly)
  // These fonts are embedded as C arrays and loaded directly from memory
  _resources.fonts["d_din"] = std::make_unique<ml::FontResource>(nvg, "d_din", resources::D_DIN_otf, resources::D_DIN_otf_size, 0);
  _resources.fonts["d_din_italic"] = std::make_unique<ml::FontResource>(nvg, "d_din_italic", resources::D_DIN_Italic_otf, resources::D_DIN_Italic_otf_size, 0);

  // Helpful for debugging layout
  // Uncomment these and `make -j` in your build directory to enable them
  // _drawingProperties.setProperty("draw_widget_bounds", true);
  // _drawingProperties.setProperty("draw_background_grid", true);
}
