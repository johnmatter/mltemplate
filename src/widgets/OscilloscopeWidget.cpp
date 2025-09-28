#include "OscilloscopeWidget.h"
#include "MLDSPProjections.h"

using namespace ml;

OscilloscopeWidget::OscilloscopeWidget(WithValues p) : Widget(p)
{
  // Initialize circular buffers
  for(int i = 0; i < kMaxChannels; ++i)
  {
    if (_buffers[i].resize(kBufferLength) != kBufferLength)
    {
      _enabled = false;
      return;
    }
    _buffers[i].clear();
  }
  _enabled = true;
}

void OscilloscopeWidget::setupParams()
{
  // Initialize parameters with defaults - called after widget creation
  _timebaseScale = getFloatPropertyWithDefault("timebase_scale", 1.0f);
  _amplitudeScale = getFloatPropertyWithDefault("amplitude_scale", 1.0f);
  _triggerLevel = getFloatPropertyWithDefault("trigger_level", 0.05f);
  _triggerChannel = getIntPropertyWithDefault("trigger_channel", 0);
  _triggerEnabled = getBoolPropertyWithDefault("trigger_enable", true);

  // CRITICAL: Call base class setupParams() - required for widget to function
  Widget::setupParams();
}

MessageList OscilloscopeWidget::animate(int elapsedTimeInMs, ml::DrawContext dc)
{
  _dirty = true;  // Force regular redraws for real-time visualization


  return MessageList{};
}

void OscilloscopeWidget::resize(ml::DrawContext dc)
{
  // Nothing special needed for resize
}

int OscilloscopeWidget::findTriggerPosition(const std::array<float, kBufferLength>& buffer, int startPos)
{
  if (!_triggerEnabled) return startPos;

  // Simple rising edge trigger: look for zero crossing
  for (int i = 1; i < kBufferLength / 2; ++i)
  {
    int pos = (startPos + i) % kBufferLength;
    int prevPos = (pos - 1 + kBufferLength) % kBufferLength;

    if (buffer[prevPos] <= _triggerLevel && buffer[pos] > _triggerLevel)
    {
      return pos;
    }
  }

  // No trigger found, return starting position
  return startPos;
}

void OscilloscopeWidget::extractChannelData(int channel, std::array<float, kBufferLength>& output)
{
  if (channel >= _channels || !_hasValidData) {
    std::fill(output.begin(), output.end(), 0.0f);
    return;
  }

  // Get most recent data from circular buffer
  _buffers[channel].peekMostRecent(output.data(), kBufferLength);
}

void OscilloscopeWidget::draw(ml::DrawContext dc)
{
  NativeDrawContext* nvg = getNativeContext(dc);
  Rect bounds = getLocalBounds(dc, *this);
  const int gridSize = dc.coords.gridSizeInPixels;

  // Check if enabled - following mlvg widget patterns
  bool enabled = getBoolPropertyWithDefault("enabled", true);
  if (!enabled) return;

  nvgSave(nvg);  // Save context like complex widgets do

  // Basic drawing setup
  const float strokeWidth = gridSize / 144.0f;
  float xMargin = gridSize / 32.0f;
  float yMargin = gridSize / 16.0f;

  // ALWAYS draw background frame - even without data
  auto frameColor = nvgRGBA(64, 64, 64, 128);  // Dark gray frame
  nvgStrokeColor(nvg, frameColor);
  nvgStrokeWidth(nvg, strokeWidth);
  nvgBeginPath(nvg);
  nvgRect(nvg, xMargin, yMargin, bounds.width() - 2*xMargin, bounds.height() - 2*yMargin);
  nvgStroke(nvg);

  // Draw center line for reference
  nvgBeginPath(nvg);
  nvgMoveTo(nvg, xMargin, bounds.center().y());
  nvgLineTo(nvg, bounds.width() - xMargin, bounds.center().y());
  nvgStrokeColor(nvg, nvgRGBA(96, 96, 96, 64));  // Lighter gray reference line
  nvgStrokeWidth(nvg, strokeWidth * 0.5f);
  nvgStroke(nvg);

  // If we have valid data, draw waveforms
  if (_enabled && _hasValidData)
  {
    float drawHeight = bounds.height() - 2 * yMargin;
    float channelHeight = drawHeight / std::max(1, _channels);

    // Set up projections
    auto sampleToX = projections::linear({0.0f, (float)(kBufferLength - 1)},
                                         {xMargin, bounds.width() - xMargin});

    // Get trigger position for stable display
    std::array<float, kBufferLength> triggerChannel;
    extractChannelData(_triggerChannel, triggerChannel);
    int triggerPos = findTriggerPosition(triggerChannel, _lastTriggerPosition);
    _lastTriggerPosition = triggerPos;

    // Draw each channel
    for (int ch = 0; ch < _channels; ++ch)
    {
      std::array<float, kBufferLength> channelData;
      extractChannelData(ch, channelData);

      // Calculate vertical position for this channel
      float channelCenterY = yMargin + (ch + 0.5f) * channelHeight;
      float maxAmplitude = channelHeight * 0.4f;

      auto amplitudeToY = projections::linear({-_amplitudeScale, _amplitudeScale},
                                             {channelCenterY + maxAmplitude, channelCenterY - maxAmplitude});

      // Begin path for this channel's waveform
      nvgBeginPath(nvg);

      bool firstPoint = true;
      for (int i = 0; i < kBufferLength; ++i)
      {
        int sampleIndex = (triggerPos + i) % kBufferLength;
        float sampleValue = channelData[sampleIndex];

        float x = sampleToX(i);
        float y = amplitudeToY(sampleValue);

        if (firstPoint)
        {
          nvgMoveTo(nvg, x, y);
          firstPoint = false;
        }
        else
        {
          nvgLineTo(nvg, x, y);
        }
      }

      // Draw the waveform in black as requested
      nvgStrokeColor(nvg, nvgRGBA(0, 0, 0, 255));
      nvgStrokeWidth(nvg, strokeWidth);
      nvgStroke(nvg);
    }
  }
  else
  {
    // No data - draw "No Signal" indicator
    auto font = getFontResource(dc, "d_din");
    if (font)
    {
      float textSize = gridSize * 0.3f;
      nvgFontFaceId(nvg, font->handle);
      nvgFontSize(nvg, textSize);
      nvgFillColor(nvg, nvgRGBA(128, 128, 128, 192));
      drawText(nvg, bounds.center(), "No Signal", NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    }
  }

  nvgRestore(nvg);  // Restore context
}

void OscilloscopeWidget::processPublishedSignal(Value sigVal, Symbol sigName)
{
  if (!_enabled) return;

  // Debug: Always print when we receive signals
  printf("Oscilloscope: Received signal '%s', size=%d bytes\n",
         sigName.getUTF8Ptr(), (int)sigVal.getBlobSize());

  // Extract interleaved channel data from the Value blob
  // This follows the same pattern as SumuScope
  float* pData = static_cast<float*>(sigVal.getBlobData());
  int sizeInFloats = sigVal.getBlobSize() / sizeof(float);

  // Debug: Check if we're receiving data
  static int debugCounter = 0;
  if (debugCounter++ % 100 == 0) // Print every 100 calls
  {
    printf("Oscilloscope: Received %d floats, first few values: ", sizeInFloats);
    for (int i = 0; i < std::min(8, sizeInFloats); ++i)
    {
      printf("%.6f ", pData[i]);
    }
    printf("\n");
  }

  // Determine number of channels and frames
  // For now, assume stereo (2 channels) but allow mono
  int detectedChannels = std::min(2, std::max(1, sizeInFloats / 64));  // Reasonable guess
  _channels = std::min(detectedChannels, kMaxChannels);

  int frames = sizeInFloats / _channels;
  frames = std::min(frames, kBufferLength);  // Don't overflow our buffers

  // Write interleaved data to separate channel buffers
  // Data format: [ch0_frame0, ch1_frame0, ch0_frame1, ch1_frame1, ...]
  for (int i = 0; i < frames; ++i)
  {
    for (int ch = 0; ch < _channels; ++ch)
    {
      if (i * _channels + ch < sizeInFloats)
      {
        float sample = pData[i * _channels + ch];
        _buffers[ch].write(&sample, 1);
      }
    }
  }

  _hasValidData = true;
  _dirty = true;  // Request redraw
}
