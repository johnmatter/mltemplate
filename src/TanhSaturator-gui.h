#pragma once

#include "CLAPExport.h"
#include "nanovg.h"
#include <string>

constexpr int kGridUnitsX{ 9 };
constexpr int kGridUnitsY{ 3 };
constexpr int kDefaultGridSize{ 60 };
constexpr int kMinGridSize{ 30 };
constexpr int kMaxGridSize{ 120 };

// Forward declaration
class TanhSaturator;

// Minimal GUI class
class TanhSaturatorGUI : public ml::CLAPAppView<TanhSaturator> {
  public:
    // Constructor
    TanhSaturatorGUI(TanhSaturator* processor);
    ~TanhSaturatorGUI() override = default;

    // Create your specific widgets
    void makeWidgets() override;

    // Helper function to layout widgets with consistent positioning
    void layoutView(ml::DrawContext dc) override;

    // Set up your visual style
    void initializeResources(NativeDrawContext* nvg) override;
};
