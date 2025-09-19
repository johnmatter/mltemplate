#include "ChordGenerator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

ChordGenerator::ChordGenerator() {
  buildParameterDescriptions();

  // Initialize per-voice parameter cache with defaults from parameter system
  for (auto& voice : voiceDSP) {
    // Initialize wavetable oscillators with saw wave (matching original SawGen behavior)
    for (int i = 0; i < kChordVoices; ++i) {
      voice.chordOscillators[i].setSawWave();
      voice.chordSineGens[i].clear();  // Initialize SineGen oscillators

      // Initialize amplitude smoothing glides (following sumu pattern)
      voice.voiceAmpGlides[i].clear();  // Clear glide state
    }

    // Get attack default from parameter description (in ms, convert to seconds)
    for (const auto& paramDesc : _params.descriptions) {
      ml::Path paramName = paramDesc->getTextProperty("name");
      if (paramName == "attack") {
        voice.lastAttack = paramDesc->getFloatProperty("plaindefault") * 1e-3f;
      } else if (paramName == "release") {
        voice.lastRelease = paramDesc->getFloatProperty("plaindefault") * 1e-3f;
      }
    }

    // Initialize amplitude glide times (following sumu pattern)
    float sr = 44100.0f;    // Default sample rate, will be updated in setSampleRate
    float glideTimeInSamples = 2.0f * sr / 1000.0f;  // 2ms
    for (int i = 0; i < kChordVoices; ++i) {
      voice.voiceAmpGlides[i].setGlideTimeInSamples(glideTimeInSamples);
    }

    // Initializate ADSR coefficients with parameter defaults
    float attack = voice.lastAttack;
    float decay = 0.1f; // TODO: might as well expose this as a parameter too
    float sustain = 1.0f; // TODO: might as well expose this as a parameter too
    float release = voice.lastRelease;
		//
    voice.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);
  }
}

void ChordGenerator::setSampleRate(double sr) {
  // Clear oscillators and reinitialize wavetables
  for (auto& voice : voiceDSP) {
    for (int i = 0; i < kChordVoices; ++i) {
      voice.chordOscillators[i].clear();
      voice.chordOscillators[i].setSawWave();  // Reinitialize wavetable
      voice.chordSineGens[i].clear();  // Clear SineGen oscillators

      // Update amplitude glides for new sample rate (2ms glide time, following sumu pattern)
      float glideTimeInSamples = 2.0f * sr / 1000.0f;  // 2ms
      voice.voiceAmpGlides[i].setGlideTimeInSamples(glideTimeInSamples);
      voice.voiceAmpGlides[i].clear();  // Reset glide state
    }

    // Update ADSR coefficients for new sample rate
    float attack = voice.lastAttack;
    float decay = 0.1f; // TODO: might as well expose this as a parameter too
    float sustain = 1.0f;   // TODO: might as well expose this as a parameter too
    float release = voice.lastRelease;

    voice.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);
  }

  // Note: Polyphony is set by the CLAP wrapper in CLAPExport.h (line 303)
  // The wrapper calls audioContext->setInputPolyphony(16) by default
  // For monophonic operation, we rely on the voice allocation logic to limit to 1 voice
}

void ChordGenerator::processVector(const ml::DSPVectorDynamic& inputs, ml::DSPVectorDynamic& outputs, void* stateData) {
  // Get AudioContext from stateData for MIDI voice access
  auto* audioContext = static_cast<ml::AudioContext*>(stateData);
  if (!audioContext) {
    outputs[0] = ml::DSPVector(0.0f);
    outputs[1] = ml::DSPVector(0.0f);
    return;
  }


  // Update chord selection and inversion once per audio frame
  float harmonics = this->getRealFloatParam("harmonics");
  float inversion = this->getRealFloatParam("inversion");
  float detune = this->getRealFloatParam("detune");
  selectChord(harmonics, detune);
  computeChordInversion(inversion);

  // Maybe a weird pattern, but I might want to allocate N voices in voiceDSP but set fewer voices using a parameter
  const int maxVoices = std::min(static_cast<int>(voiceDSP.size()), audioContext->getInputPolyphony());

  // Each voice gets added to this DSPVector
  ml::DSPVector totalOutput{0.0f};

  // Process voices
  for (int v = 0; v < maxVoices; ++v) {
    auto& voice = const_cast<ml::EventsToSignals::Voice&>(audioContext->getInputVoice(v));
    ml::DSPVector voiceOutput = processVoice(v, voice, audioContext);
    totalOutput += voiceOutput;
  }

  // DC blocking is now applied per-oscillator before envelope to prevent
  // DC blocker from fighting against ADSR envelope dynamics

  // Gain for sum of voice outputs
  // Note: DC blocking is now applied per-oscillator before envelope,
  // so gain compensation is less aggressive
  float level = this->getRealFloatParam("level");
  float totalGain = 1.5f * level;  // Back to original gain since DC blocking is more targeted

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
  float inversion = inversionParam * float(kNotesPerChord * kChordVoices);  // Scale by notes * chord voices

  // Extract integral and fractional parts (using MAKE_INTEGRAL_FRACTIONAL pattern)
  int inversionIntegral = static_cast<int>(inversion);
  float inversionFractional = inversion - static_cast<float>(inversionIntegral);

  int numRotations = inversionIntegral / kNotesPerChord;
  int rotatedNote = inversionIntegral % kNotesPerChord;

  const float kBaseGain = 0.25f;

  // Initialize all chord voices to zero - they will be set by the algorithm below
  for (int i = 0; i < kChordVoices; ++i) {
    chordState.voiceAmplitudes[i] = 0.0f;
    chordState.voiceRatios[i] = 1.0f;  // Default to unity ratio
  }

  // Process each chord note and assign to voices
  for (int i = 0; i < kNotesPerChord; ++i) {
    float transposition = 0.25f * static_cast<float>(
        1 << ((kNotesPerChord - 1 + inversionIntegral - i) / kNotesPerChord));
    int targetVoice = (i - numRotations + kChordVoices) % kChordVoices;
    int previousVoice = (targetVoice - 1 + kChordVoices) % kChordVoices;

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
    {"plaindefault", 0.5f}, // Start with root position
    {"units", ""}
  }));

  // Output parameter - overall level
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "level"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.8f},
    {"units", ""}
  }));

  // Amplitude parameter - per-chord-voice level
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

  // Attack parameter in milliseconds
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "attack"},
    {"range", {0.0f, 1000.0f}},
    {"plaindefault", 10.0f},
    {"units", "ms"}
  }));

  // Release parameter in milliseconds
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "release"},
    {"range", {00.0f, 2000.0f}},
    {"plaindefault", 500.0f},
    {"units", "ms"}
  }));

  // Debug oscillator switch - 0=Wavetable, 1=SineGen
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "debug_osc"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.0f},  // Default to wavetable
    {"units", ""}
  }));

  // DC blocker enable/disable
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "dc_block"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 1.0f},  // Default to enabled
    {"units", ""}
  }));

  this->buildParams(params);
  this->setDefaultParams();
}

ml::DSPVector ChordGenerator::processVoice(int voiceIndex, ml::EventsToSignals::Voice& voice, ml::AudioContext* audioContext) {
  // Map voiceIndex to our single voiceDSP slot using modulo
  // This allows the host to allocate 16 voices but we only use 1 physical voice
  int mappedVoiceIndex = voiceIndex % kNumVoices;

  // Bounds check to prevent crashes
  if (mappedVoiceIndex < 0 || mappedVoiceIndex >= voiceDSP.size()) {
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

  // Chord parameters are updated once per frame in processVector()

  // Convert MIDI pitch to Hz: 440 * 2^((note-69)/12)
  const ml::DSPVector vPitchOffset = vPitch - ml::DSPVector(69.0f);
  const ml::DSPVector vPitchRatio = pow(ml::DSPVector(2.0f), vPitchOffset * ml::DSPVector(1.0f/12.0f));
  const ml::DSPVector vFreqHz = ml::DSPVector(440.0f) * vPitchRatio;

  // Get ADSR parameters
  float attack = this->getRealFloatParam("attack") * 1e-3;
  float decay = 0.1f;     // 100ms decay
  float sustain = 1.0f;   // 100% sustain level
  float release = this->getRealFloatParam("release") * 1e-3;

  // Only update ADSR coefficients if parameters changed (to avoid resetting envelope state)
  VoiceDSP& voiceDsp = voiceDSP[mappedVoiceIndex];
  if (attack != voiceDsp.lastAttack || release != voiceDsp.lastRelease) {
    voiceDsp.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);
    voiceDsp.lastAttack = attack;
    voiceDsp.lastRelease = release;
  }

  // Get amplitude parameter
  float amplitude = this->getRealFloatParam("amplitude");

  // Get debug oscillator switch (0=Wavetable, 1=SineGen)
  float debugOsc = this->getRealFloatParam("debug_osc");
  bool useSineGen = debugOsc > 0.5f;  // Comparator at 0.5

  // Process ADSR envelope using gate signal scaled by amplitude
  // ml::ADSR expects gate+amp signal: the input value becomes both trigger and amplitude scaling
  const ml::DSPVector vGateWithAmp = vGate * ml::DSPVector(amplitude);
  const ml::DSPVector vEnvelope = voiceDSP[mappedVoiceIndex].mADSR(vGateWithAmp);

  // Generate full chord using the chord synthesis algorithm
  ml::DSPVector chordOutput{0.0f};

  for (int chordVoice = 0; chordVoice < kChordVoices; ++chordVoice) {
    // Get target amplitude from chord state and smooth it to prevent zippering (following sumu pattern)
    float targetAmp = chordState.voiceAmplitudes[chordVoice];
    ml::DSPVector smoothedAmp = voiceDSP[mappedVoiceIndex].voiceAmpGlides[chordVoice](targetAmp);

    // Calculate frequency for this chord voice using the chord ratios
    const ml::DSPVector chordFreq = vFreqHz * ml::DSPVector(chordState.voiceRatios[chordVoice]);
    const ml::DSPVector vFreqNorm = chordFreq / ml::DSPVector(sr);

    // Generate oscillator output - switch between wavetable and sine based on debug parameter
    ml::DSPVector vOscillator;
    if (useSineGen) {
      // Use simple sine wave generator for debugging
      vOscillator = voiceDSP[mappedVoiceIndex].chordSineGens[chordVoice](vFreqNorm);
    } else {
      // Use wavetable oscillator (saw wave by default)
      vOscillator = voiceDSP[mappedVoiceIndex].chordOscillators[chordVoice](vFreqNorm);
    }

    // Apply smoothed amplitude first, then DC block the raw oscillator signal
    // This prevents DC blocker from fighting against envelope dynamics
    ml::DSPVector vOscWithAmp = vOscillator * smoothedAmp;

    // DC block the raw oscillator signal before envelope application
    // Use per-voice DC blocker to maintain proper state per oscillator
    float dcBlockEnabled = this->getRealFloatParam("dc_block");
    if (dcBlockEnabled > 0.5f) {
      float sr = audioContext->getSampleRate();
      voiceDSP[mappedVoiceIndex].voiceDCBlockers[chordVoice].mCoeffs = ml::DCBlocker::coeffs(200.0f / sr);

      // Reset DC blocker when oscillator type changes to prevent energy buildup
      // This prevents DC blocker from being "confused" by sudden wavetable->sine transitions
      // Use per-voice tracking to handle polyphonic scenarios correctly
      if (useSineGen != (voiceDSP[mappedVoiceIndex].lastDebugOsc > 0.5f)) {
        // Reset DC blocker by creating a fresh instance
        // This clears the internal state (x1, y1) that was adapted to the previous oscillator type
        voiceDSP[mappedVoiceIndex].voiceDCBlockers[chordVoice] = ml::DCBlocker();
        voiceDSP[mappedVoiceIndex].voiceDCBlockers[chordVoice].mCoeffs = ml::DCBlocker::coeffs(200.0f / sr);
        voiceDSP[mappedVoiceIndex].lastDebugOsc = debugOsc;
      }

      vOscWithAmp = voiceDSP[mappedVoiceIndex].voiceDCBlockers[chordVoice](vOscWithAmp);
    }

    // Apply envelope to the DC-blocked signal
    const ml::DSPVector vOutput = vOscWithAmp * vEnvelope;
    chordOutput += vOutput;
  }

  return chordOutput;
}
