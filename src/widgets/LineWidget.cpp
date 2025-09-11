#include "LineWidget.h"

using namespace ml;

void LineWidget::draw(ml::DrawContext dc)
{
  NativeDrawContext* nvg = getNativeContext(dc);
  Rect bounds = getLocalBounds(dc, *this);
  
  bool enabled = getBoolPropertyWithDefault("enabled", true);
  if(!enabled) return;
  
  // Get line properties
  auto color = getColorPropertyWithDefault("color", getColor(dc, "mark"));
  float thickness = getFloatPropertyWithDefault("thickness", 1.0f);
  float opacity = getFloatPropertyWithDefault("opacity", 1.0f);
  
  // Apply opacity to color
  auto lineColor = multiplyAlpha(color, opacity);
  
  // Draw the line from left to right, centered vertically
  nvgBeginPath(nvg);
  nvgMoveTo(nvg, bounds.left(), bounds.center().y());
  nvgLineTo(nvg, bounds.right(), bounds.center().y());
  nvgStrokeColor(nvg, lineColor);
  nvgStrokeWidth(nvg, thickness);
  nvgStroke(nvg);
}
