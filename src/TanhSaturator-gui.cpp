#include "TanhSaturator-gui.h"
#include "TanhSaturator.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// Include embedded font resources
#include "../build/resources/TanhSaturator/resources.c"
#include "../build/font_resources/TanhSaturator/resources.c"

// Constructor - plugin-specific implementation
TanhSaturatorGUI::TanhSaturatorGUI(TanhSaturator* processor)
  : CLAPAppView("TanhSaturator", processor) {

  // Set up grid system for fixed aspect ratio
  setGridSizeDefault(kDefaultGridSize);
  setGridSizeLimits(kMinGridSize, kMaxGridSize);
  setFixedAspectRatio({kGridUnitsX, kGridUnitsY});
}

// Pure virtual override from CLAPAppView
void TanhSaturatorGUI::makeWidgets() {

  _view->_widgets.add_unique<TextLabelBasic>("title", ml::WithValues{
    {"bounds", {0.02*kGridUnitsX, 0.0, 0.8*kGridUnitsX, 1.0}},
    {"text", "TanhSaturator"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("title_text_size")},
    {"h_align", "left"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Input gain
  _view->_widgets.add_unique<DialBasic>("input", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("input_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "input"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("input_label", ml::WithValues{
    {"text", "in"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // Output gain
  _view->_widgets.add_unique<DialBasic>("output", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("output_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "output"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("output_label", ml::WithValues{
    {"text", "out"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // Dry/Wet mix
  _view->_widgets.add_unique<DialBasic>("dry_wet", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("dry_wet_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "dry_wet"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("dry_wet_label", ml::WithValues{
    {"text", "mix"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // lowpass frequency
  _view->_widgets.add_unique<DialBasic>("lowpass", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("lowpass_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "lowpass"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("lowpass_label", ml::WithValues{
    {"text", "lpf"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // Lowpass Q parameter
  _view->_widgets.add_unique<DialBasic>("lowpass_q", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("lowpass_q_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_bounds"),
                _drawingProperties.getFloatProperty("dial_bounds")}},
    {"size", _drawingProperties.getFloatProperty("dial_size")},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "lowpass_q"}
  });

  _view->_widgets.add_unique<TextLabelBasic>("lowpass_q_label", ml::WithValues{
    {"text", "q"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })},
    {"bounds", {0, 0, 1.0, 0.3}}
  });

  // horizontal separator line
  _view->_widgets.add_unique<LineWidget>("separator_line", ml::WithValues{
    {"bounds", {0.1, 0.4, 8.8, 1.0}},  // x, y, width, height
    {"color", ml::colorToMatrix({ 0.3, 0.3, 0.3, 1.0 })},  // gray color
    {"thickness", 4.0f},  // 2 pixel thick line
    {"opacity", 0.8f}     // 80% opacity
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
void TanhSaturatorGUI::layoutView(ml::DrawContext dc) {

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

  // Position TanhSaturator dials
  positionLabelUnderDial("input", "input_label");
  positionLabelUnderDial("output", "output_label");
  positionLabelUnderDial("dry_wet", "dry_wet_label");
  positionLabelUnderDial("lowpass", "lowpass_label");
  positionLabelUnderDial("lowpass_q", "lowpass_q_label");

}

// Pure virtual override from CLAPAppView - must implement to set up fonts, colors, and layout
void TanhSaturatorGUI::initializeResources(NativeDrawContext* nvg) {
  if (!nvg) return;

  // Set up visual style for this plugin
  _drawingProperties.setProperty("mark", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 }));
  _drawingProperties.setProperty("mark_bright", ml::colorToMatrix({ 0.9, 0.9, 0.9, 1.0 }));
  _drawingProperties.setProperty("background", ml::colorToMatrix({ 0.6, 0.7, 0.8, 1.0}));
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

  // Column positions for five dials in one row
  float dialBounds = _drawingProperties.getFloatProperty("dial_bounds");
  float totalWidth = kGridUnitsX;
  float spacing = (totalWidth - 5 * dialBounds) / 6.0f; // Equal spacing between dials and edges
  
  _drawingProperties.setProperty("input_dial_x", spacing * 1 + dialBounds * 0);
  _drawingProperties.setProperty("output_dial_x", spacing * 2 + dialBounds * 1);
  _drawingProperties.setProperty("lowpass_dial_x", spacing * 3 + dialBounds * 2);
  _drawingProperties.setProperty("lowpass_q_dial_x", spacing * 4 + dialBounds * 3);
  _drawingProperties.setProperty("dry_wet_dial_x", spacing * 5 + dialBounds * 4);

  // Load embedded fonts (essential for text to work properly)
  // These fonts are embedded as C arrays and loaded directly from memory
  _resources.fonts["d_din"] = std::make_unique<ml::FontResource>(nvg, "d_din", resources::D_DIN_otf, resources::D_DIN_otf_size, 0);
  _resources.fonts["d_din_italic"] = std::make_unique<ml::FontResource>(nvg, "d_din_italic", resources::D_DIN_Italic_otf, resources::D_DIN_Italic_otf_size, 0);

  // Helpful for debugging layout
  // Uncomment these and `make -j` in your build directory to enable them
  // _drawingProperties.setProperty("draw_widget_bounds", true);
  // _drawingProperties.setProperty("draw_background_grid", true);
}
