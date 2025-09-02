#include "TapeHack-gui.h"
#include "TapeHack.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// Include embedded font resources
#include "../build/resources/TapeHack/resources.c"
#include "../build/font_resources/TapeHack/resources.c"

ClapStereoEffectTemplateGUI::ClapStereoEffectTemplateGUI(ClapStereoEffectTemplate* processor)
  : CLAPAppView("ClapStereoEffectTemplate", processor) {

  // Set up grid system for fixed aspect ratio (following MLVG pattern)
  setGridSizeDefault(kDefaultGridSize);
  setGridSizeLimits(kMinGridSize, kMaxGridSize);
  setFixedAspectRatio({kGridUnitsX, kGridUnitsY});
}

void ClapStereoEffectTemplateGUI::makeWidgets() {

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("title", ml::WithValues{
    {"bounds", {0, 0.5, kGridUnitsX, 0.5}},
    {"text", "TapeHack"},
    {"font", "montserrat"},
    {"text_size", _drawingProperties.getFloatProperty("title_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Input gain
  _view->_widgets.add_unique<DialBasic>("input", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("input_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_size"),
                _drawingProperties.getFloatProperty("dial_size")}},
    {"size", 1.0f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "input"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("input_label", ml::WithValues{
    {"text", "in"},
    {"font", "montserrat"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Output gain
  _view->_widgets.add_unique<DialBasic>("output", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("output_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_size"),
                _drawingProperties.getFloatProperty("dial_size")}},
    {"size", 1.0f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "output"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("output_label", ml::WithValues{
    {"text", "out"},
    {"font", "montserrat"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Dry/Wet mix
  _view->_widgets.add_unique<DialBasic>("dry_wet", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("dry_wet_dial_x"),
                _drawingProperties.getFloatProperty("dial_row_y"),
                _drawingProperties.getFloatProperty("dial_size"),
                _drawingProperties.getFloatProperty("dial_size")}},
    {"size", 1.0f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "dry_wet"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("dry_wet_label", ml::WithValues{
    {"text", "mix"},
    {"font", "montserrat"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Add resize widget to bottom right corner
  _view->_widgets.add_unique<Resizer>("resizer", ml::WithValues{
    {"fix_ratio", static_cast<float>(kGridUnitsX)/static_cast<float>(kGridUnitsY)},  // Use grid constants for aspect ratio
    {"z", -2},                  // Stay on top of other widgets
    {"fixed_size", true},
    {"fixed_bounds", {-16, -16, 16, 16}},  // 16x16 pixel resize handle
    {"anchor", {1, 1}}          // Anchor to bottom right corner {1, 1}
  });
}

void ClapStereoEffectTemplateGUI::layoutView(ml::DrawContext dc) {

  // Helper function to position labels under dials consistently
  auto positionLabelUnderDial = [&](ml::Path dialName, ml::Path labelName) {
    // Safety check: ensure both widgets exist before accessing them
    if (!_view->_widgets[dialName] || !_view->_backgroundWidgets[labelName]) {
      return;
    }
    
    // Get the actual bounds of both widgets
    ml::Rect dialRect = _view->_widgets[dialName]->getRectProperty("bounds");
    ml::Rect labelRect = _view->_backgroundWidgets[labelName]->getRectProperty("bounds");
    
    // Safety check: ensure we have valid bounds
    if (dialRect.width() <= 0 || dialRect.height() <= 0) {
      return;
    }
    
    // Calculate the center of the dial
    ml::Vec2 dialCenter = dialRect.center();
    
    // Calculate the desired position for the label (centered horizontally under the dial)
    // Use a small gap between dial and label (0.2 grid units)
    float gap = -0.2f;
    float labelWidth = std::max(labelRect.width(), 2.0f);  // Ensure minimum width
    float labelHeight = std::max(labelRect.height(), 0.4f); // Ensure minimum height
    
    ml::Vec2 labelPosition = ml::Vec2(
      dialCenter.x() - labelWidth * 0.5f,  // Center horizontally
      dialRect.bottom() + gap               // Position below dial with gap
    );
    
    // Ensure the label stays within reasonable bounds (0 to grid size)
    labelPosition.x() = std::max(0.0f, std::min(labelPosition.x(), kGridUnitsX - labelWidth));
    labelPosition.y() = std::max(0.0f, std::min(labelPosition.y(), kGridUnitsY - labelHeight));
    
    // Set the label bounds to the calculated position
    ml::Rect newLabelBounds(labelPosition.x(), labelPosition.y(), labelWidth, labelHeight);
    _view->_backgroundWidgets[labelName]->setRectProperty("bounds", newLabelBounds);
  };

  // Position TapeHack dials
  positionLabelUnderDial("input", "input_label");
  positionLabelUnderDial("output", "output_label");
  positionLabelUnderDial("dry_wet", "dry_wet_label");
}

void ClapStereoEffectTemplateGUI::initializeResources(NativeDrawContext* nvg) {
  if (!nvg) return;

  // Set up visual style for this plugin
  _drawingProperties.setProperty("mark", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 }));
  _drawingProperties.setProperty("mark_bright", ml::colorToMatrix({ 0.9, 0.9, 0.9, 1.0 }));
  _drawingProperties.setProperty("background", ml::colorToMatrix({ 0.6, 0.7, 0.8, 1.0 }));
  _drawingProperties.setProperty("common_stroke_width", 1 / 32.f);

  // Centralized typography
  _drawingProperties.setProperty("title_text_size", 0.8f);
  _drawingProperties.setProperty("label_text_size", 0.4f);
  _drawingProperties.setProperty("dial_text_size", 0.5f);

  // Single dial size for all dials
  _drawingProperties.setProperty("dial_size", 1.8f);

  // Single row for all dials
  _drawingProperties.setProperty("dial_row_y", 1.5f);

  // Column positions for three dials in one row
  float dialSize = _drawingProperties.getFloatProperty("dial_size");
  float totalWidth = kGridUnitsX;
  float spacing = (totalWidth - 3 * dialSize) / 4.0f; // Equal spacing between dials and edges
  
  _drawingProperties.setProperty("input_dial_x",
                                 spacing * 1 + dialSize * 0);
  _drawingProperties.setProperty("output_dial_x",
                                 spacing * 2 + dialSize * 1);
  _drawingProperties.setProperty("dry_wet_dial_x", 
                                 spacing * 3 + dialSize * 2);

  // Load embedded fonts (essential for text to work properly)
  // These fonts are embedded as C arrays and loaded directly from memory
  _resources.fonts["d_din"] = std::make_unique<ml::FontResource>(nvg, "d_din", resources::D_DIN_otf, resources::D_DIN_otf_size, 0);
  _resources.fonts["d_din_italic"] = std::make_unique<ml::FontResource>(nvg, "d_din_italic", resources::D_DIN_Italic_otf, resources::D_DIN_Italic_otf_size, 0);

  // Load custom fonts
  _resources.fonts["astloch_regular"] = std::make_unique<ml::FontResource>(nvg, "astloch_regular", resources::Astloch_Regular_ttf, resources::Astloch_Regular_ttf_size, 0);
  _resources.fonts["astloch_bold"] = std::make_unique<ml::FontResource>(nvg, "astloch_bold", resources::Astloch_Bold_ttf, resources::Astloch_Bold_ttf_size, 0);
  _resources.fonts["odibee_sans"] = std::make_unique<ml::FontResource>(nvg, "odibee_sans", resources::OdibeeSans_Regular_ttf, resources::OdibeeSans_Regular_ttf_size, 0);
  _resources.fonts["almendra_display"] = std::make_unique<ml::FontResource>(nvg, "almendra_display", resources::AlmendraDisplay_Regular_ttf, resources::AlmendraDisplay_Regular_ttf_size, 0);
  _resources.fonts["montserrat"] = std::make_unique<ml::FontResource>(nvg, "montserrat", resources::Montserrat_Regular_ttf, resources::Montserrat_Regular_ttf_size, 0);

  // Helpful for debugging layout
  // _drawingProperties.setProperty("draw_widget_bounds", true);
  // _drawingProperties.setProperty("draw_background_grid", true);

}
