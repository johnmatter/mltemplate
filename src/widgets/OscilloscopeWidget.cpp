#include "OscilloscopeWidget.h"
#include <algorithm>
#include <mutex>

OscilloscopeWidget::OscilloscopeWidget(ml::WithValues p) 
  : Widget(p) {
  waveformData.reserve(kMaxSamples);
}

void OscilloscopeWidget::draw(ml::DrawContext dc) {
  NativeDrawContext* nvg = getNativeContext(dc);
  if (!nvg) return;
  
  ml::Rect bounds = getLocalBounds(dc, *this);
  
  if (bounds.width() <= 0 || bounds.height() <= 0) return;
  
  // Draw background
  drawBackground(dc, bounds);
  
  // Draw grid if enabled
  if (showGrid) {
    drawGrid(dc, bounds);
  }
  
  // Draw center line
  drawCenterLine(dc, bounds);
  
  // Draw waveform
  drawWaveform(dc, bounds);
}

ml::MessageList OscilloscopeWidget::animate(int elapsedTimeInMs, ml::DrawContext dc) {
  // Mark as dirty to trigger redraw
  _dirty = true;
  return ml::MessageList{};
}

void OscilloscopeWidget::updateWaveform(const std::vector<float>& samples) {
  std::lock_guard<std::mutex> lock(waveformMutex);
  
  waveformData.clear();
  waveformData.reserve(std::min(samples.size(), kMaxSamples));
  
  // Copy samples, limiting to max size
  size_t copySize = std::min(samples.size(), kMaxSamples);
  waveformData.assign(samples.begin(), samples.begin() + copySize);
}

void OscilloscopeWidget::setWaveformColor(const ml::Matrix& color) {
  waveformColor = color;
}

void OscilloscopeWidget::setBackgroundColor(const ml::Matrix& color) {
  backgroundColor = color;
}

void OscilloscopeWidget::setGridVisible(bool visible) {
  showGrid = visible;
}

void OscilloscopeWidget::drawBackground(ml::DrawContext dc, const ml::Rect& bounds) {
  NativeDrawContext* nvg = getNativeContext(dc);
  
  nvgBeginPath(nvg);
  nvgRect(nvg, bounds.left(), bounds.top(), bounds.width(), bounds.height());
  nvgFillColor(nvg, nvgRGBAf(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]));
  nvgFill(nvg);
}

void OscilloscopeWidget::drawGrid(ml::DrawContext dc, const ml::Rect& bounds) {
  NativeDrawContext* nvg = getNativeContext(dc);
  
  nvgStrokeWidth(nvg, 0.5f);
  nvgStrokeColor(nvg, nvgRGBAf(gridColor[0], gridColor[1], gridColor[2], gridColor[3] * 0.5f));
  
  // Vertical grid lines
  int numVerticalLines = 8;
  for (int i = 1; i < numVerticalLines; ++i) {
    float x = bounds.left() + (bounds.width() * i / numVerticalLines);
    nvgBeginPath(nvg);
    nvgMoveTo(nvg, x, bounds.top());
    nvgLineTo(nvg, x, bounds.bottom());
    nvgStroke(nvg);
  }
  
  // Horizontal grid lines
  int numHorizontalLines = 6;
  for (int i = 1; i < numHorizontalLines; ++i) {
    float y = bounds.top() + (bounds.height() * i / numHorizontalLines);
    nvgBeginPath(nvg);
    nvgMoveTo(nvg, bounds.left(), y);
    nvgLineTo(nvg, bounds.right(), y);
    nvgStroke(nvg);
  }
}

void OscilloscopeWidget::drawCenterLine(ml::DrawContext dc, const ml::Rect& bounds) {
  NativeDrawContext* nvg = getNativeContext(dc);
  
  float centerY = (bounds.top() + bounds.bottom()) * 0.5f;
  
  nvgStrokeWidth(nvg, 1.0f);
  nvgStrokeColor(nvg, nvgRGBAf(gridColor[0], gridColor[1], gridColor[2], gridColor[3] * 0.8f));
  
  nvgBeginPath(nvg);
  nvgMoveTo(nvg, bounds.left(), centerY);
  nvgLineTo(nvg, bounds.right(), centerY);
  nvgStroke(nvg);
}

void OscilloscopeWidget::drawWaveform(ml::DrawContext dc, const ml::Rect& bounds) {
  NativeDrawContext* nvg = getNativeContext(dc);
  
  std::lock_guard<std::mutex> lock(waveformMutex);
  
  if (waveformData.empty()) return;
  
  nvgStrokeWidth(nvg, 2.0f);
  nvgStrokeColor(nvg, nvgRGBAf(waveformColor[0], waveformColor[1], waveformColor[2], waveformColor[3]));
  
  nvgBeginPath(nvg);
  
  size_t numSamples = waveformData.size();
  for (size_t i = 0; i < numSamples; ++i) {
    float x = bounds.left() + (bounds.width() * i / (numSamples - 1));
    
    // Clamp sample to prevent drawing outside bounds
    float sample = std::clamp(waveformData[i], -1.0f, 1.0f);
    float centerY = (bounds.top() + bounds.bottom()) * 0.5f;
    float y = centerY - (sample * bounds.height() * 0.4f);
    
    if (i == 0) {
      nvgMoveTo(nvg, x, y);
    } else {
      nvgLineTo(nvg, x, y);
    }
  }
  
  nvgStroke(nvg);
}
