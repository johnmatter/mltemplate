#include "Oscilloscope.h"
#include <algorithm>
#include "MLDSPMath.h"

Oscilloscope::Oscilloscope() {
  buildParameterDescriptions();
  
  // Initialize oscilloscope buffer
  oscilloscopeBuffer.resize(kOscilloscopeBufferSize, 0.0f);
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
  
  // Copy all 64 samples from the DSPVector to our circular buffer
  for (int i = 0; i < kFloatsPerDSPVector; ++i) {
    oscilloscopeBuffer[oscilloscopeWriteIndex] = leftChannel[i];
    oscilloscopeWriteIndex = (oscilloscopeWriteIndex + 1) % kOscilloscopeBufferSize;
  }
}


void Oscilloscope::updateEffectState() {
  // Oscilloscope is always active for visualization
  isActive = true;
}

void Oscilloscope::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;
  this->buildParams(params);
  this->setDefaultParams();
}

std::vector<float> Oscilloscope::getOscilloscopeData() const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(oscilloscopeMutex));
  
  // Create a copy of the buffer data
  std::vector<float> result;
  result.reserve(kOscilloscopeBufferSize);
  
  // Copy data starting from current write position to get most recent samples
  for (size_t i = 0; i < kOscilloscopeBufferSize; ++i) {
    size_t index = (oscilloscopeWriteIndex + i) % kOscilloscopeBufferSize;
    result.push_back(oscilloscopeBuffer[index]);
  }
  
  return result;
}
