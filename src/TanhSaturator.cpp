#include "TanhSaturator.h"
#include <algorithm>
#include <cmath>

TanhSaturator::TanhSaturator() {
  // buildParameterDescriptions() sets up the plugin's parameter system:
  // - Defines parameter names, ranges, default values, and units
  // - Creates the parameter tree that hosts can query and automate
  // - Establishes the connection between GUI controls and DSP parameters
  // This is called in the constructor because parameters must be defined
  // before the plugin can be used by hosts.
  buildParameterDescriptions();
  
  // For simple stateless effects like tanh saturation, no additional
  // initialization is needed here. More complex effects might:
  // - Initialize filter coefficients or delay line sizes
  // - Set up modulation sources (LFOs, envelopes)
  // - Allocate memory for buffers or lookup tables
  // - Configure effect-specific constants or lookup tables
  // Note: Sample-rate dependent initialization should be done in setSampleRate()
}

void TanhSaturator::setSampleRate(double sr) {
  // setSampleRate() is called by the host when the plugin is loaded or
  // when the project sample rate changes. This is where you should:
  // - Initialize filter coefficients that depend on sample rate
  // - Set up delay line sizes or buffer lengths
  // - Configure time-based parameters (LFO rates, envelope times)
  // - Recalculate any sample-rate dependent constants
  // 
  // For simple stateless effects like tanh saturation, no sample rate
  // dependent initialization is needed.
}

void TanhSaturator::processAudioContext() {
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

void TanhSaturator::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Get effect parameters
  float inputGain = this->getRealFloatParam("input") * 3.0f;  // Reduced gain for tanh
  float outputGain = this->getRealFloatParam("output");
  float wet = this->getRealFloatParam("dry_wet");

  // Store dry samples for wet/dry mix
  ml::DSPVector dryLeft = leftChannel;
  ml::DSPVector dryRight = rightChannel;

  // Process left channel
  leftChannel = processTanhSaturation(leftChannel, inputGain, outputGain);
  
  // Process right channel
  rightChannel = processTanhSaturation(rightChannel, inputGain, outputGain);
  
  // Apply wet/dry mix
  leftChannel = (leftChannel * wet) + (dryLeft * (1.0f - wet));
  rightChannel = (rightChannel * wet) + (dryRight * (1.0f - wet));
}

ml::DSPVector TanhSaturator::processTanhSaturation(const ml::DSPVector& inputSamples, float inputGain, float outputGain) {
  // Apply input gain
  ml::DSPVector processed = inputSamples * inputGain;
  
  // Apply tanh saturation
  // tanh(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
  // madronalib provides a SIMD-optimized exp() function
  ml::DSPVector expPos = exp(processed);
  ml::DSPVector expNeg = exp(ml::DSPVector(0.0f) - processed);
  processed = (expPos - expNeg) / (expPos + expNeg);
  
  // Apply output gain
  processed *= outputGain;
  
  return processed;
}

void TanhSaturator::updateEffectState() {
  // Determine if effect is active based on parameters
  float inputGain = this->getRealFloatParam("input");
  float outputGain = this->getRealFloatParam("output");
  float wet = this->getRealFloatParam("dry_wet");
  
  const float activityThreshold = 0.001f;
  isActive = (inputGain > activityThreshold || outputGain > activityThreshold || wet > activityThreshold);
}

void TanhSaturator::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;

  // Input gain parameter (A from original)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "input"},
    {"range", {0.0f, 10.0f}},
    {"plaindefault", 0.5f},
    {"units", ""}
  }));

  // Output gain parameter (B from original)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "output"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  // Dry/Wet mix parameter (C from original)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "dry_wet"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  this->buildParams(params);

  // Set default parameter values after building
  this->setDefaultParams();
}
