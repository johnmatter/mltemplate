#include "TanhSaturator.h"
#include <algorithm>
#include <cmath>

// Constructor - plugin-specific implementation
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

// Override from CLAPSignalProcessor - called by CLAP host when sample rate changes
void TanhSaturator::setSampleRate(double sr) {
  // setSampleRate() is called by the host when the plugin is loaded or
  // when the project sample rate changes. This is where you should:
  // - Initialize filter coefficients that depend on sample rate
  // - Set up delay line sizes or buffer lengths
  // - Configure time-based parameters (LFO rates, envelope times)
  // - Recalculate any sample-rate dependent constants
  
  // Initialize lowpass filters following CLAP saw demo pattern
  // Use the parameter system's default value (defined in buildParameterDescriptions)
  float defaultFreq = this->getRealFloatParam("lowpass");
  float defaultQ = this->getRealFloatParam("lowpass_q");
  
  // Lopass::makeCoeffs expects k = 1/Q, where k=0 is maximum resonance
  float filterK = 1.0f / defaultQ;
  effectState.lowpassL._coeffs = ml::Lopass::makeCoeffs(defaultFreq / sr, filterK);
  effectState.lowpassR._coeffs = ml::Lopass::makeCoeffs(defaultFreq / sr, filterK);
}

// Plugin-specific implementation - called by CLAPExport.h for each audio block
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

// Helper method - plugin-specific DSP processing
void TanhSaturator::processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel) {
  // Get effect parameters
  float inputGain = this->getRealFloatParam("input");
  float outputGain = this->getRealFloatParam("output");
  float lowpassFreq = this->getRealFloatParam("lowpass");
  float lowpassQ = this->getRealFloatParam("lowpass_q");
  float dryWetMix = this->getRealFloatParam("dry_wet");

  // Store original input for dry signal
  ml::DSPVector inputLeft = leftChannel;
  ml::DSPVector inputRight = rightChannel;
  
  // Step 1: Apply tanh saturation to both channels
  leftChannel = processTanhSaturation(leftChannel, inputGain, outputGain);
  rightChannel = processTanhSaturation(rightChannel, inputGain, outputGain);
  
  // Step 2: Apply post-saturation lowpass filtering using Lopass filters
  const float sr = audioContext->getSampleRate();
  float normalizedFreq = lowpassFreq / sr;
  normalizedFreq = std::min(normalizedFreq, 0.45f);  // clamp below nyquist; we might not need this
  
  // Lopass::makeCoeffs expects k = 1/Q, where k=0 is maximum resonance
  float filterK = 1.0f / lowpassQ;
  effectState.lowpassL._coeffs = ml::Lopass::makeCoeffs(normalizedFreq, filterK);
  effectState.lowpassR._coeffs = ml::Lopass::makeCoeffs(normalizedFreq, filterK);
  
  // Step 3: lowpass filter the saturated signals
  leftChannel = effectState.lowpassL(leftChannel);
  rightChannel = effectState.lowpassR(rightChannel);
  
  // Step 4: Apply dry/wet mix
  ml::DSPVector wetNorm = ml::DSPVector(dryWetMix);
  ml::DSPVector dryMix = ml::DSPVector(1.0f) - wetNorm * wetNorm;
  ml::DSPVector wetMix = ml::DSPVector(1.0f) - dryMix;
  
  // Mix dry and wet signals
  leftChannel = inputLeft * dryMix + leftChannel * wetMix;
  rightChannel = inputRight * dryMix + rightChannel * wetMix;
}

// Helper method - plugin-specific tanh saturation algorithm
ml::DSPVector TanhSaturator::processTanhSaturation(const ml::DSPVector& inputSamples, float inputGain, float outputGain) {
  // Apply input gain
  ml::DSPVector processed = inputSamples * inputGain;
  
  // Apply tanh saturation
  // tanh(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
  // madronalib provides a SIMD-optimized exp() function
  ml::DSPVector expPos = exp(processed);
  ml::DSPVector expNeg = exp(ml::DSPVector(0.0f) - processed); // madronalib doesn't currently have a unary minus operator for DSPVectors, so we need to subtract from a DSPVector's worth of 0.0f
  processed = (expPos - expNeg) / (expPos + expNeg);
  
  // Apply output gain
  processed *= outputGain;
  
  return processed;
}

// Helper method - updates plugin activity state for CLAP sleep/continue
void TanhSaturator::updateEffectState() {
  // Determine if effect is active based on parameters
  float inputGain = this->getRealFloatParam("input");
  float outputGain = this->getRealFloatParam("output");
  float dryWetMix = this->getRealFloatParam("dry_wet");
  float lowpassQ = this->getRealFloatParam("lowpass_q");
  
  const float activityThreshold = 0.001f;
  isActive = (inputGain > activityThreshold || outputGain > activityThreshold || dryWetMix > activityThreshold || lowpassQ > activityThreshold);
}

// Plugin-specific implementation - defines parameters using madronalib ParameterTree system
void TanhSaturator::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;

  // Input gain
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "input"},
    {"range", {0.0f, 5.0f}},
    {"plaindefault", 2.2f},
    {"units", ""}
  }));

  // Output gain
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "output"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.8f},
    {"units", ""}
  }));

  // Dry/Wet mix
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "dry_wet"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  // Lowpass frequency parameter
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "lowpass"},
    {"range", {50.0f, 20000.0f}},
    {"plaindefault", 1700.0f},
    {"units", "Hz"},
    {"log", true}  // Use logarithmic scaling for frequency
  }));

  // Lowpass Q parameter (resonance)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "lowpass_q"},
    {"range", {0.1f, 10.0f}},
    {"plaindefault", 2.2f},
    {"units", ""}
  }));

  this->buildParams(params);

  // Set default parameter values after building
  this->setDefaultParams();
}
