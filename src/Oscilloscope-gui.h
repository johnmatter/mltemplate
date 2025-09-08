#pragma once

#include "CLAPExport.h"
#include "nanovg.h"
#include <string>

constexpr int kGridUnitsX{ 8 };
constexpr int kGridUnitsY{ 6 }; // Increased to accommodate oscilloscope
constexpr int kDefaultGridSize{ 60 };
constexpr int kMinGridSize{ 30 };
constexpr int kMaxGridSize{ 120 };

// Forward declaration
class Oscilloscope;
class OscilloscopeWidget;

// Minimal GUI class
class OscilloscopeGUI : public ml::CLAPAppView<Oscilloscope> {
  public:
    // Constructor
    OscilloscopeGUI(Oscilloscope* processor);
    ~OscilloscopeGUI() override = default;

    // Create your specific widgets
    void makeWidgets() override;

    // Layout widgets with consistent positioning
    void layoutView(ml::DrawContext dc) override;

    // Set up your visual style
    void initializeResources(NativeDrawContext* nvg) override;

    // Update oscilloscope data
    void updateOscilloscope();

    // Override animate to update oscilloscope
    void animate(NativeDrawContext* nvg) override;

private:
    OscilloscopeWidget* oscilloscopeWidget = nullptr;
    int lastOscilloscopeUpdate = 0;
};
