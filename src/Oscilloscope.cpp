#include "Oscilloscope.h"
#include <algorithm>
#include "MLDSPMath.h"

Oscilloscope::Oscilloscope() {
  buildParameterDescriptions();
  
  // Initialize oscilloscope buffer
  oscilloscopeBuffer.resize(oscilloscopeBufferSize, 0.0f);
}

void Oscilloscope::setSampleRate(double sr) {
  // Initialize effect state with sample rate
  // No sample rate dependent initialization needed for TapeHack
}

void Oscilloscope::processAudioContext() {
  // Safety check - ensure AudioContext is valid
  if (!audioContext) {
    return;
  }

  // Get input channels
  ml::DSPVector leftInput = audioContext->inputs[0];
  ml::DSPVector rightInput = audioContext->inputs[1];
  
  // Create output buffers
  ml::DSPVector leftOutput = leftInput;
  ml::DSPVector rightOutput = rightInput;

  // Process the stereo effect
  processStereoEffect(leftOutput, rightOutput);
  
  // Update effect state
  updateEffectState();
  
  // Set outputs
  audioContext->outputs[0] = leftOutput;
  audioContext->outputs[1] = rightOutput;
}

void Oscilloscope::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Update oscilloscope buffer with all samples from the DSPVector
  std::lock_guard<std::mutex> lock(oscilloscopeMutex);
  
  // Get buffer size from parameter (convert from index to actual size)
  float bufferSizeIndex = getRealFloatParam("buffer_size");
  size_t newBufferSize = 64 << static_cast<int>(bufferSizeIndex); // 64 * 2^index
  
  // Update buffer size if parameter changed
  if (newBufferSize != oscilloscopeBufferSize) {
    oscilloscopeBufferSize = newBufferSize;
    oscilloscopeBuffer.resize(oscilloscopeBufferSize, 0.0f);
    oscilloscopeWriteIndex = 0; // Reset write index
  }
  
  // Copy all 64 samples from the DSPVector to our circular buffer
  for (int i = 0; i < kFloatsPerDSPVector; ++i) {
    oscilloscopeBuffer[oscilloscopeWriteIndex] = leftChannel[i];
    oscilloscopeWriteIndex = (oscilloscopeWriteIndex + 1) % oscilloscopeBufferSize;
  }
}


void Oscilloscope::updateEffectState() {
  // Oscilloscope is always active for visualization
  isActive = true;
}

void Oscilloscope::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;
  
  // Buffer size parameter (powers of 2: 64, 128, 256, 512, 1024, 2048)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "buffer_size"},
    {"range", {0.0f, 5.0f}}, // 0=64, 1=128, 2=256, 3=512, 4=1024, 5=2048
    {"plaindefault", 2.0f}, // Default to 256 samples (index 2)
    {"units", "samples"},
    {"display_scale", 1.0f},
    {"display_offset", 64.0f} // Will be converted to actual size in getRealFloatParam
  }));
  
  this->buildParams(params);
  this->setDefaultParams();
}

std::vector<float> Oscilloscope::getOscilloscopeData() const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(oscilloscopeMutex));
  
  // Create a copy of the buffer data
  std::vector<float> result;
  result.reserve(oscilloscopeBufferSize);
  
  // Copy data starting from current write position to get most recent samples
  for (size_t i = 0; i < oscilloscopeBufferSize; ++i) {
    size_t index = (oscilloscopeWriteIndex + i) % oscilloscopeBufferSize;
    result.push_back(oscilloscopeBuffer[index]);
  }
  
  return result;
}

