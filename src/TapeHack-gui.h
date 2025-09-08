#pragma once

#include "CLAPExport.h"
#include "nanovg.h"
#include <string>

constexpr int kGridUnitsX{ 8 };
constexpr int kGridUnitsY{ 4 };
constexpr int kDefaultGridSize{ 60 };
constexpr int kMinGridSize{ 30 };
constexpr int kMaxGridSize{ 120 };

// Forward declaration
class TapeHack;

// Minimal GUI class
class TapeHackGUI : public ml::CLAPAppView<TapeHack> {
  public:
    // Constructor
    TapeHackGUI(TapeHack* processor);
    ~TapeHackGUI() override = default;

    // Create your specific widgets
    void makeWidgets() override;

    // Layout widgets with consistent positioning
    void layoutView(ml::DrawContext dc) override;

    // Set up your visual style
    void initializeResources(NativeDrawContext* nvg) override;
};
