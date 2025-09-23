#pragma once

#include "CLAPExport.h"
#include "widgets/LineWidget.h"
#include "widgets/OscilloscopeWidget.h"

constexpr int kGridUnitsX{ 9 };
constexpr int kGridUnitsY{ 5 };
constexpr int kDefaultGridSize{ 40 };
constexpr int kMinGridSize{ 30 };
constexpr int kMaxGridSize{ 120 };

// Forward declaration
class ChordGenerator;

// Minimal GUI class
class ChordGeneratorGUI : public ml::CLAPAppView<ChordGenerator> {
  public:
    // Constructor
    ChordGeneratorGUI(ChordGenerator* processor);
    ~ChordGeneratorGUI() override = default;

    // Create your specific widgets
    void makeWidgets() override;

    // Helper function to layout widgets with consistent positioning
    void layoutView(ml::DrawContext dc) override;

    // Set up your visual style
    void initializeResources(NativeDrawContext* nvg) override;
};
