#pragma once

#include "MLWidget.h"

using namespace ml;

class LineWidget : public Widget
{  
public:
  LineWidget(WithValues p) : Widget(p) {}

  // Widget implementation
  void draw(ml::DrawContext dc) override;
};
