#include "clap-stereo-effect-template-gui.h"
#include "clap-stereo-effect-template.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// Include embedded font resources
#include "../build/resources/clap-stereo-effect-template/resources.c"

ClapStereoEffectTemplateGUI::ClapStereoEffectTemplateGUI(ClapStereoEffectTemplate* processor)
  : CLAPAppView("ClapStereoEffectTemplate", processor) {

  // Set up grid system for fixed aspect ratio (following MLVG pattern)
  setGridSizeDefault(kDefaultGridSize);
  setGridSizeLimits(kMinGridSize, kMaxGridSize);
  setFixedAspectRatio({kGridUnitsX, kGridUnitsY});
}

void ClapStereoEffectTemplateGUI::makeWidgets() {

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("title", ml::WithValues{
    {"bounds", {0, 0.2, kGridUnitsX, 0.5}},
    {"text", "Stereo Effect Template"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("title_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Main gain
  _view->_widgets.add_unique<DialBasic>("gain", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("center_col_x"),
                _drawingProperties.getFloatProperty("top_row_y"),
                _drawingProperties.getFloatProperty("large_dial_size"),
                _drawingProperties.getFloatProperty("large_dial_size")}},
    {"size", 1.0f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "gain"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("gain_label", ml::WithValues{
    {"text", "Main"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Left gain
  _view->_widgets.add_unique<DialBasic>("left_gain", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("left_col_x"),
                _drawingProperties.getFloatProperty("bottom_row_y"),
                _drawingProperties.getFloatProperty("small_dial_size"),
                _drawingProperties.getFloatProperty("small_dial_size")}},
    {"size", 0.8f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "left_gain"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("left_gain_label", ml::WithValues{
    {"text", "Left"},
    {"font", "d_din"},
    {"text_size", _drawingProperties.getFloatProperty("label_text_size")},
    {"h_align", "center"},
    {"v_align", "middle"},
    {"text_color", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 })}
  });

  // Right gain
  _view->_widgets.add_unique<DialBasic>("right_gain", ml::WithValues{
    {"bounds", {_drawingProperties.getFloatProperty("right_col_x"),
                _drawingProperties.getFloatProperty("bottom_row_y"),
                _drawingProperties.getFloatProperty("small_dial_size"),
                _drawingProperties.getFloatProperty("small_dial_size")}},
    {"size", 0.8f},
    {"visible", true},
    {"draw_number", true},
    {"text_size", _drawingProperties.getFloatProperty("dial_text_size")},
    {"param", "right_gain"}
  });

  _view->_backgroundWidgets.add_unique<TextLabelBasic>("right_gain_label", ml::WithValues{
    {"text", "Right"},
    {"font", "d_din"},
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

  // Position gain dials
  positionLabelUnderDial("gain", "gain_label");
  positionLabelUnderDial("left_gain", "left_gain_label");
  positionLabelUnderDial("right_gain", "right_gain_label");
}

void ClapStereoEffectTemplateGUI::initializeResources(NativeDrawContext* nvg) {
  if (!nvg) return;

  // Set up visual style for this plugin
  _drawingProperties.setProperty("mark", ml::colorToMatrix({ 0.01, 0.01, 0.01, 1.0 }));
  _drawingProperties.setProperty("mark_bright", ml::colorToMatrix({ 0.9, 0.9, 0.9, 1.0 }));
  _drawingProperties.setProperty("background", ml::colorToMatrix({ 0.6, 0.7, 0.8, 1.0 }));
  _drawingProperties.setProperty("common_stroke_width", 1 / 32.f);

  // Centralized typography
  _drawingProperties.setProperty("title_text_size", 0.3f);
  _drawingProperties.setProperty("label_text_size", 0.4f);
  _drawingProperties.setProperty("dial_text_size", 0.5f);

  // Dial sizes (circular dials)
  _drawingProperties.setProperty("large_dial_size", 2.0f);
  _drawingProperties.setProperty("small_dial_size", 2.0f);

  // Row positions
  _drawingProperties.setProperty("top_row_y", 0.8f);
  _drawingProperties.setProperty("bottom_row_y", 1.2f);

  // Column positions
  float offset = 2.5f;
  _drawingProperties.setProperty(
    "left_col_x",
    kGridUnitsX/2.0
    - offset
    - _drawingProperties.getFloatProperty("small_dial_size")/2.0
  );
  _drawingProperties.setProperty(
    "center_col_x",
    kGridUnitsX/2.0
    - _drawingProperties.getFloatProperty("large_dial_size")/2.0
  );
  _drawingProperties.setProperty(
    "right_col_x",
    kGridUnitsX/2.0
    + offset
    - _drawingProperties.getFloatProperty("small_dial_size")/2.0
  );

  // Load embedded fonts (essential for text to work properly)
  // These fonts are embedded as C arrays and loaded directly from memory
  _resources.fonts["d_din"] = std::make_unique<ml::FontResource>(nvg, "d_din", resources::D_DIN_otf, resources::D_DIN_otf_size, 0);
  _resources.fonts["d_din_italic"] = std::make_unique<ml::FontResource>(nvg, "d_din_italic", resources::D_DIN_Italic_otf, resources::D_DIN_Italic_otf_size, 0);

  // Helpful for debugging layout
  // _drawingProperties.setProperty("draw_widget_bounds", true);
  // _drawingProperties.setProperty("draw_background_grid", true);

}
