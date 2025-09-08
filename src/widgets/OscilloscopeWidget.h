#pragma once

#include "CLAPExport.h"
#include "nanovg.h"
#include <vector>
#include <array>

class OscilloscopeWidget : public ml::Widget {
public:
  OscilloscopeWidget(ml::WithValues p);
  ~OscilloscopeWidget() override = default;

  // Widget interface
  void draw(ml::DrawContext dc) override;
  ml::MessageList animate(int elapsedTimeInMs, ml::DrawContext dc) override;

  // Oscilloscope interface
  void updateWaveform(const std::vector<float>& samples);
  void setWaveformColor(const ml::Matrix& color);
  void setBackgroundColor(const ml::Matrix& color);
  void setGridVisible(bool visible);

private:
  void drawBackground(ml::DrawContext dc, const ml::Rect& bounds);
  void drawGrid(ml::DrawContext dc, const ml::Rect& bounds);
  void drawWaveform(ml::DrawContext dc, const ml::Rect& bounds);
  void drawCenterLine(ml::DrawContext dc, const ml::Rect& bounds);

  // Waveform data
  std::vector<float> waveformData;
  static constexpr size_t kMaxSamples = 128; // 2 DSPVectors worth of samples
  
  // Visual properties
  ml::Matrix waveformColor{ml::colorToMatrix({0.0f, 1.0f, 0.0f, 1.0f})}; // Green
  ml::Matrix backgroundColor{ml::colorToMatrix({0.1f, 0.1f, 0.1f, 1.0f})}; // Dark gray
  ml::Matrix gridColor{ml::colorToMatrix({0.3f, 0.3f, 0.3f, 1.0f})}; // Light gray
  bool showGrid{true};
  
  // Thread safety
  std::mutex waveformMutex;
};
