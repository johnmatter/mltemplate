#include "clap-stereo-effect-template.h"
#include <algorithm>

ClapStereoEffectTemplate::ClapStereoEffectTemplate() {
  buildParameterDescriptions();
}

void ClapStereoEffectTemplate::setSampleRate(double sr) {
  // Initialize effect state with sample rate
  // Add your effect-specific initialization here
}

void ClapStereoEffectTemplate::processAudioContext() {
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
  
  // Apply main gain
  float mainGain = this->getRealFloatParam("gain");
  leftOutput *= ml::DSPVector(mainGain);
  rightOutput *= ml::DSPVector(mainGain);
  
  // Set outputs
  audioContext->outputs[0] = leftOutput;
  audioContext->outputs[1] = rightOutput;
}

void ClapStereoEffectTemplate::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Get effect parameters
  float leftGain = this->getRealFloatParam("left_gain");
  float rightGain = this->getRealFloatParam("right_gain");
  
  // Apply stereo gains
  leftChannel *= ml::DSPVector(leftGain);
  rightChannel *= ml::DSPVector(rightGain);
  
  // Store state for activity detection
  effectState.leftGain = leftGain;
  effectState.rightGain = rightGain;
}

void ClapStereoEffectTemplate::updateEffectState() {
  // Determine if effect is active based on parameters and audio
  float mainGain = this->getRealFloatParam("gain");
  
  // Effect is active if any gain is above threshold
  const float activityThreshold = 0.001f;
  isActive = (mainGain > activityThreshold) || 
             (effectState.leftGain > activityThreshold) || 
             (effectState.rightGain > activityThreshold);
}

void ClapStereoEffectTemplate::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;

  // Main gain parameter
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "gain"},
    {"range", {0.0f, 2.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  // Stereo gain parameters
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "left_gain"},
    {"range", {0.0f, 2.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "right_gain"},
    {"range", {0.0f, 2.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  this->buildParams(params);

  // Set default parameter values after building
  this->setDefaultParams();
}

// All CLAP boilerplate methods moved to CLAPSignalProcessor base class
