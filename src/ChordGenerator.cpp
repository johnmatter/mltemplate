#include "ChordGenerator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

ChordGenerator::ChordGenerator() {
  buildParameterDescriptions();
}

void ChordGenerator::setSampleRate(double sr) {
  // Clear oscillators
  for (auto& voice : voiceDSP) {
    for (int i = 0; i < 5; ++i) {
      voice.chordOscillators[i].clear();
    }
  }
}

void ChordGenerator::processVector(const ml::DSPVectorDynamic& inputs, ml::DSPVectorDynamic& outputs, void* stateData) {
  // Get AudioContext from stateData for MIDI voice access
  auto* audioContext = static_cast<ml::AudioContext*>(stateData);
  if (!audioContext) {
    outputs[0] = ml::DSPVector(0.0f);
    outputs[1] = ml::DSPVector(0.0f);
    return;
  }

  // Reset voice activity counter
  activeVoiceCount = 0;

  // Process all voices - the ADSR envelope silences inactive ones
  const int maxVoices = std::min(static_cast<int>(voiceDSP.size()), audioContext->getInputPolyphony());

  ml::DSPVector totalOutput{0.0f};

  for (int v = 0; v < maxVoices; ++v) {
    // Safety check for voice access
    if (v >= audioContext->getInputPolyphony()) {
      break;
    }

    auto& voice = const_cast<ml::EventsToSignals::Voice&>(audioContext->getInputVoice(v));

    // Process this voice with chord synthesis
    ml::DSPVector voiceOutput = processVoice(v, voice, audioContext);

    // Track activity using voice output (includes ADSR envelope)
    float voiceLevel = ml::sum(voiceOutput * voiceOutput);
    const float silenceThreshold = 1e-6f;
    if (voiceLevel > silenceThreshold) {
      activeVoiceCount++;
      totalOutput += voiceOutput;
    }
  }

  // Apply overall amplitude parameter
  float masterGain = this->getRealFloatParam("amplitude");

  // Normalize by voice count to prevent clipping
  float voiceNormalization = (activeVoiceCount > 0) ? (1.0f / std::sqrt(activeVoiceCount)) : 1.0f;

  // Apply final gain
  float totalGain = 0.5f * voiceNormalization * masterGain;

  // Set outputs
  outputs[0] = totalOutput * ml::DSPVector(totalGain);
  outputs[1] = totalOutput * ml::DSPVector(totalGain);
}

// Helper method - chord selection algorithm with cycling and detuning
void ChordGenerator::selectChord(float harmonicsParam, float detuneCents) {
  float scaled = harmonicsParam * 1.02f;

  // quantize to chord index with hysteresis to prevent jitter
  int chordIndex = static_cast<int>(scaled * (kNumChords - 1.001f));
  chordIndex = std::max(0, std::min(chordIndex, kNumChords - 1));

  // update if chord changed
  if (chordIndex != chordState.currentChord) {
    chordState.currentChord = chordIndex;
  }
  
  // Always update the base chord ratios (separate from voice processing)
  // Store base chord ratios with detuning - these are used by the inversion algorithm
  for (int i = 0; i < kNotesPerChord; ++i) {
    // Base frequency ratio from chord
    float baseRatio = semitonesToRatio(chords_[chordIndex][i]);
    
    // Simple detuning: slight variations per voice
    float detuneMultiplier = (i == 0) ? 0.0f : (i - 1.5f) * 0.5f; // -0.75, -0.25, +0.25, +0.75
    
    // Apply detuning: cents to ratio conversion
    float detuneRatio = std::pow(2.0f, (detuneCents * detuneMultiplier) / 1200.0f);
    
    // Store base ratios - these are the chord intervals before inversion processing
    chordState.baseChordRatios[i] = baseRatio * detuneRatio;
  }
}

// Helper method - chord inversion algorithm from Plaits
void ChordGenerator::computeChordInversion(float inversionParam) {
  // Based on the original Plaits ComputeChordInversion implementation
  const float* baseRatios = chordState.baseChordRatios;  // Use immutable base chord ratios
  float inversion = inversionParam * float(kNotesPerChord * kNumVoices);  // Scale by notes * voices

  // Extract integral and fractional parts (using MAKE_INTEGRAL_FRACTIONAL pattern)
  int inversionIntegral = static_cast<int>(inversion);
  float inversionFractional = inversion - static_cast<float>(inversionIntegral);
  
  int numRotations = inversionIntegral / kNotesPerChord;
  int rotatedNote = inversionIntegral % kNotesPerChord;
  
  const float kBaseGain = 0.25f;
  
  // Initialize all voices to zero - they will be set by the algorithm below
  for (int i = 0; i < kNumVoices; ++i) {
    chordState.voiceAmplitudes[i] = 0.0f;
    chordState.voiceRatios[i] = 1.0f;  // Default to unity ratio
  }
  
  // Process each chord note and assign to voices
  for (int i = 0; i < kNotesPerChord; ++i) {
    float transposition = 0.25f * static_cast<float>(
        1 << ((kNotesPerChord - 1 + inversionIntegral - i) / kNotesPerChord));
    int targetVoice = (i - numRotations + kNumVoices) % kNumVoices;
    int previousVoice = (targetVoice - 1 + kNumVoices) % kNumVoices;
    
    if (i == rotatedNote) {
      // Crossfade between current and next octave for smooth inversion
      chordState.voiceRatios[targetVoice] = baseRatios[i] * transposition;
      chordState.voiceRatios[previousVoice] = baseRatios[i] * transposition * 2.0f;  // One octave up
      chordState.voiceAmplitudes[previousVoice] = kBaseGain * inversionFractional;
      chordState.voiceAmplitudes[targetVoice] = kBaseGain * (1.0f - inversionFractional);
    } else if (i < rotatedNote) {
      // Notes below the rotated note go to previous voice  
      chordState.voiceRatios[previousVoice] = baseRatios[i] * transposition;
      chordState.voiceAmplitudes[previousVoice] = kBaseGain;
    } else {
      // Notes above the rotated note go to target voice
      chordState.voiceRatios[targetVoice] = baseRatios[i] * transposition;
      chordState.voiceAmplitudes[targetVoice] = kBaseGain;
    }
  }
  
  // Ensure at least one voice remains active (safety fallback)
  float totalAmplitude = 0.0f;
  for (int i = 0; i < kNumVoices; ++i) {
    totalAmplitude += chordState.voiceAmplitudes[i];
  }
  
  if (totalAmplitude < 0.01f) {
    // Emergency fallback: activate the root voice with proper ratio
    chordState.voiceAmplitudes[0] = kBaseGain;
    chordState.voiceRatios[0] = baseRatios[0];  // Use actual root note ratio
  }
}

void ChordGenerator::buildParameterDescriptions() {
  ml::ParameterDescriptionList params;

  // Harmonics parameter - selects chord type (0-1 maps to chord bank)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "harmonics"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.9f},  // Default to Major chord (last in bank)
    {"units", ""}
  }));

  // Inversion parameter - controls chord inversion and voicing (0-1)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "inversion"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.0f},  // Start with root position
    {"units", ""}
  }));

  // Amplitude parameter - overall output level
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "amplitude"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.5f},
    {"units", ""}
  }));

  // Detune parameter in cents 
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "detune"},
    {"range", {0.0f, 50.0f}},
    {"plaindefault", 8.0f},
    {"units", "cents"}
  }));

  this->buildParams(params);
  this->setDefaultParams();
}

ml::DSPVector ChordGenerator::processVoice(int voiceIndex, ml::EventsToSignals::Voice& voice, ml::AudioContext* audioContext) {
  // Bounds check to prevent crashes
  if (voiceIndex < 0 || voiceIndex >= voiceDSP.size()) {
    return ml::DSPVector{0.0f};
  }

  // Safety check for AudioContext
  if (!audioContext) {
    return ml::DSPVector{0.0f};
  }

  // Get voice control signals provided by EventsToSignals
  const ml::DSPVector vPitch = voice.outputs.row(ml::kPitch);
  const ml::DSPVector vGate = voice.outputs.row(ml::kGate);
  
  const float sr = audioContext->getSampleRate();
  if (sr <= 0.0f) {
    return ml::DSPVector{0.0f};
  }

  // Get chord synthesis parameters
  float harmonics = this->getRealFloatParam("harmonics");
  float inversion = this->getRealFloatParam("inversion"); 
  float detune = this->getRealFloatParam("detune");

  // Update chord selection with detuning and inversion
  selectChord(harmonics, detune);
  computeChordInversion(inversion);

  // Convert MIDI pitch to Hz: 440 * 2^((note-69)/12)
  const ml::DSPVector vPitchOffset = vPitch - ml::DSPVector(69.0f);
  const ml::DSPVector vPitchRatio = pow(ml::DSPVector(2.0f), vPitchOffset * ml::DSPVector(1.0f/12.0f));
  const ml::DSPVector vFreqHz = ml::DSPVector(440.0f) * vPitchRatio;

  // Get ADSR parameters (use simple defaults for now; TODO: make these parameters)
  float attack = 0.01f;   // 10ms attack
  float decay = 0.1f;     // 100ms decay  
  float sustain = 0.7f;   // 70% sustain level
  float release = 0.5f;   // 500ms release

  // Update ADSR coefficients
  voiceDSP[voiceIndex].mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);

  // Process ADSR envelope using gate signal
  const ml::DSPVector vEnvelope = voiceDSP[voiceIndex].mADSR(vGate);
  
  // Generate full chord using the chord synthesis algorithm
  ml::DSPVector chordOutput{0.0f};
  
  for (int chordVoice = 0; chordVoice < 5; ++chordVoice) {
    float voiceAmp = chordState.voiceAmplitudes[chordVoice];
    if (voiceAmp < 0.001f) continue;  // Skip silent voices
    
    // Calculate frequency for this chord voice using the chord ratios
    const ml::DSPVector chordFreq = vFreqHz * ml::DSPVector(chordState.voiceRatios[chordVoice]);
    const ml::DSPVector vFreqNorm = chordFreq / ml::DSPVector(sr);
    
    // Generate oscillator output for this chord voice
    const ml::DSPVector vOscillator = voiceDSP[voiceIndex].chordOscillators[chordVoice](vFreqNorm);
    
    // Apply voice amplitude and envelope
    const ml::DSPVector vOutput = vOscillator * vEnvelope * ml::DSPVector(voiceAmp * 0.25f);
    chordOutput += vOutput;
  }

  return chordOutput;
}
