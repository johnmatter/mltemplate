#include "TapeHack.h"
#include <algorithm>
#include <random>

TapeHack::TapeHack() {
  buildParameterDescriptions();
  
  // Initialize dither state with random values (using uint32_t like original)
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
  
  // Initialize fpd vectors with random values
  for (int i = 0; i < kFloatsPerDSPVector; ++i) {
    effectState.fpdL[i] = static_cast<float>(dis(gen));
    effectState.fpdR[i] = static_cast<float>(dis(gen));
  }
}

void TapeHack::setSampleRate(double sr) {
  // Initialize effect state with sample rate
  // No sample rate dependent initialization needed for TapeHack
}

void TapeHack::processAudioContext() {
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

void TapeHack::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Get effect parameters
  float inputGain = this->getRealFloatParam("input") * 10.0f;
  float outputGain = this->getRealFloatParam("output") * 0.9239f;
  float wet = this->getRealFloatParam("dry_wet");

  // Store dry samples for wet/dry mix
  ml::DSPVector dryLeft = leftChannel;
  ml::DSPVector dryRight = rightChannel;

  // Process left channel
  leftChannel = processTapeHackSaturation(leftChannel, inputGain, outputGain, effectState.fpdL);
  
  // Process right channel
  rightChannel = processTapeHackSaturation(rightChannel, inputGain, outputGain, effectState.fpdR);
  
  // Apply wet/dry mix
  leftChannel = (leftChannel * wet) + (dryLeft * (1.0f - wet));
  rightChannel = (rightChannel * wet) + (dryRight * (1.0f - wet));
}

ml::DSPVector TapeHack::processTapeHackSaturation(const ml::DSPVector& inputSamples, float inputGain, float outputGain, ml::DSPVector& fpd) {
  // Apply input gain and clamp to saturation range
  ml::DSPVector processed = inputSamples * inputGain;
  processed = clamp(processed, ml::DSPVector(-2.305929007734908f), ml::DSPVector(2.305929007734908f));
  
  // Apply Taylor series saturation (degenerate form to approximate sin())
  ml::DSPVector addtwo = processed * processed;
  ml::DSPVector empower = processed * addtwo; // inputSample to the third power
  processed -= (empower / 6.0f);
  empower *= addtwo; // to the fifth power
  processed += (empower / 69.0f);
  empower *= addtwo; // seventh
  processed -= (empower / 2530.08f);
  empower *= addtwo; // ninth
  processed += (empower / 224985.6f);
  empower *= addtwo; // eleventh
  processed -= (empower / 9979200.0f);
  
  // Apply output gain
  processed *= outputGain;
  
  // Dithering
  // For now, we need to process each sample individually since madronalib
  // doesn't have a SIMD XOR. The dithering differs slightly from airwindows
  for (int i = 0; i < kFloatsPerDSPVector; ++i) {
    // Convert float to uint32_t for XOR 
    uint32_t fpdInt = static_cast<uint32_t>(fpd[i]);
    
    // XOR dithering
    fpdInt ^= fpdInt << 13; 
    fpdInt ^= fpdInt >> 17; 
    fpdInt ^= fpdInt << 5;
    
    // Convert back to float
    fpd[i] = static_cast<float>(fpdInt);
    
    // Add noise to this sample in processed vector
    float ditherNoise = (static_cast<float>(fpdInt) - 2147483647.5f) * 5.5e-36f;
    processed[i] += ditherNoise;
  }
  
  return processed;
}

void TapeHack::updateEffectState() {
  // Determine if effect is active based on parameters
  float inputGain = this->getRealFloatParam("input");
  float outputGain = this->getRealFloatParam("output");
  float wet = this->getRealFloatParam("dry_wet");
  
  const float activityThreshold = 0.001f;
  isActive = (inputGain > activityThreshold || outputGain > activityThreshold || wet > activityThreshold);
}

void TapeHack::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;

  // Input gain parameter (A from original)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "input"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.1f},
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
