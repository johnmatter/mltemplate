#pragma once

#include "MLWidget.h"
#include "MLValue.h"
#include "MLDSPBuffer.h"

using namespace ml;

class OscilloscopeWidget : public Widget
{
  static constexpr int kBufferLength{ 128 };
  static constexpr int kMaxChannels{ 2 };

  // Circular buffers for storing incoming audio data
  std::array<DSPBuffer, kMaxChannels> _buffers;

  // Temporary buffer for drawing - stores one complete trace
  std::array<float, kBufferLength> _tempBuffer;

  // Trigger system
  float _triggerLevel{0.0f};
  int _triggerChannel{0};
  bool _triggerEnabled{true};
  int _triggerPosition{0};
  int _lastTriggerPosition{0};

  // Display parameters
  float _timebaseScale{1.0f};    // How much time to display
  float _amplitudeScale{1.0f};   // Amplitude scaling
  int _channels{2};              // Number of channels to display
  bool _enabled{false};

  // Internal state
  int _writePosition{0};
  bool _hasValidData{false};

  // Helper functions
  int findTriggerPosition(const std::array<float, kBufferLength>& buffer, int startPos);
  void extractChannelData(int channel, std::array<float, kBufferLength>& output);

public:
  OscilloscopeWidget(WithValues p);

  // Widget implementation
  void setupParams() override;
  void draw(ml::DrawContext d) override;
  void resize(ml::DrawContext d) override;
  MessageList animate(int elapsedTimeInMs, ml::DrawContext dc) override;
  void processPublishedSignal(Value sigVal, Symbol sigName) override;

  // Parameter handling
  bool knowsParam(Path paramName) override
  {
    if(paramName == "scope/timebase") return true;
    if(paramName == "scope/amplitude") return true;
    if(paramName == "scope/trigger_level") return true;
    if(paramName == "scope/trigger_channel") return true;
    if(paramName == "scope/trigger_enable") return true;
    return false;
  }
};
