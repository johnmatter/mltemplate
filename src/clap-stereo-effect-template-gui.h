#pragma once

#include "CLAPExport.h"
#include "nanovg.h"
#include <string>

constexpr int kGridUnitsX{ 10 };
constexpr int kGridUnitsY{ 5 };
constexpr int kDefaultGridSize{ 60 };
constexpr int kMinGridSize{ 30 };
constexpr int kMaxGridSize{ 120 };

// Forward declaration
class ClapStereoEffectTemplate;

// Minimal GUI class
class ClapStereoEffectTemplateGUI : public ml::CLAPAppView<ClapStereoEffectTemplate> {
  public:
    // Constructor
    ClapStereoEffectTemplateGUI(ClapStereoEffectTemplate* processor);
    ~ClapStereoEffectTemplateGUI() override = default;

    // Create your specific widgets
    void makeWidgets() override;

    // Layout widgets with consistent positioning
    void layoutView(ml::DrawContext dc) override;

    // Set up your visual style
    void initializeResources(NativeDrawContext* nvg) override;
};
