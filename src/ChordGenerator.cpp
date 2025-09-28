#include "ChordGenerator.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdio>  // For printf debugging
#include <chrono>  // For milliseconds
using namespace std::chrono;


ChordGenerator::ChordGenerator() {
  buildParameterDescriptions();
  
  // Initialize published signals for GUI widgets - CRITICAL for signal routing!
  // publishSignal(name, maxFrames, maxVoices, channels, octavesDown)
  this->publishSignal("scope_output", 64, 1, 2, 0);  // 64 samples, 1 voice, 2 channels, no downsampling
  

  // Initialize per-voice parameter cache with defaults from parameter system
  for (auto& voice : voiceDSP) {
    for (int i = 0; i < kChordVoices; ++i) {
      voice.chordOscillators[i].initializeGaussianWavePacket(0.5f, 0.2f, 6.0f);
      voice.chordOscillators[i].setQuantumParams(
        this->getRealFloatParam("quantum_timestep"),
        this->getRealFloatParam("quantum_mass"),
        this->getRealFloatParam("quantum_hbar")
      );
      voice.chordOscillators[i].setDecoherenceStrength(this->getRealFloatParam("quantum_decoherence"));
      voice.chordOscillators[i].setSmoothingStrength(this->getRealFloatParam("quantum_smoothing"));
      voice.chordSineGens[i].clear();
      voice.voiceAmpGlides[i].clear();
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

    // TODO: can we access the AudioContext at this point?
    float sr = 44100.0f;

    // Initializate ADSR coefficients
    // TODO: do we need this to be accurate? do we need to do it at all?
    float attack = 0.0f;
    float decay = 0.1f;
    float sustain = 1.0f;
    float release = 0.0f;
    voice.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);

    // Initialize amplitude glide times
    float glideTimeInSamples = 2.0f * sr / 1000.0f;  // 2ms
    for (int i = 0; i < kChordVoices; ++i) {
      voice.voiceAmpGlides[i].setGlideTimeInSamples(glideTimeInSamples);
    }
  }
}

ChordGenerator::~ChordGenerator() {
  // Default destructor
}

void ChordGenerator::setSampleRate(double sr) {
  // Clear oscillators and reinitialize quantum wavetables
  for (auto& voice : voiceDSP) {
    for (int i = 0; i < kChordVoices; ++i) {
      voice.chordOscillators[i].clear();
      voice.chordSineGens[i].clear();
      voice.voiceAmpGlides[i].clear();

      voice.chordOscillators[i].initializeGaussianWavePacket(0.5f, 0.2f, 6.0f);  // center, width, momentum
      voice.chordOscillators[i].setQuantumParams(
        this->getRealFloatParam("quantum_timestep"),
        this->getRealFloatParam("quantum_mass"),
        this->getRealFloatParam("quantum_hbar")
      );
      voice.chordOscillators[i].setDecoherenceStrength(this->getRealFloatParam("quantum_decoherence"));
      voice.chordOscillators[i].setSmoothingStrength(this->getRealFloatParam("quantum_smoothing"));

      float glideTimeInSamples = 2.0f * sr / 1000.0f;  // 2ms
      voice.voiceAmpGlides[i].setGlideTimeInSamples(glideTimeInSamples);
    }

    // Update ADSR coefficients for new sample rate
    float attack = voice.lastAttack;
    float decay = 0.1f;
    float sustain = 1.0f;
    float release = voice.lastRelease;

    voice.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);
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

  // Update chord selection and inversion once per audio frame
  float harmonics = this->getRealFloatParam("harmonics");
  float inversion = this->getRealFloatParam("inversion");
  float detune = this->getRealFloatParam("detune");
  selectChord(harmonics, detune);
  computeChordInversion(inversion);

  // the wavetable continues evolving even when no notes are playing
  if constexpr (kEnableQuantumSimulation) {
    // Get current quantum parameters once per audio frame
    float quantumMass = this->getRealFloatParam("quantum_mass");
    float quantumDecoherence = this->getRealFloatParam("quantum_decoherence");
    float quantumSmoothing = this->getRealFloatParam("quantum_smoothing");
    float quantumHbar = this->getRealFloatParam("quantum_hbar");
    float quantumTimestep = this->getRealFloatParam("quantum_timestep");

    for (auto& voiceDsp : voiceDSP) {
      // Check if quantum parameters have changed and update if needed
      if (quantumMass != voiceDsp.lastQuantumMass ||
          quantumDecoherence != voiceDsp.lastQuantumDecoherence ||
          quantumSmoothing != voiceDsp.lastQuantumSmoothing ||
          quantumHbar != voiceDsp.lastQuantumHbar ||
          quantumTimestep != voiceDsp.lastQuantumTimestep) {

        // Update all oscillators with new parameters
        for (auto& quantumOsc : voiceDsp.chordOscillators) {
          quantumOsc.setQuantumParams(quantumTimestep, quantumMass, quantumHbar);
          quantumOsc.setDecoherenceStrength(quantumDecoherence);
          quantumOsc.setSmoothingStrength(quantumSmoothing);
        }

        // Cache the new parameter values
        voiceDsp.lastQuantumMass = quantumMass;
        voiceDsp.lastQuantumDecoherence = quantumDecoherence;
        voiceDsp.lastQuantumSmoothing = quantumSmoothing;
        voiceDsp.lastQuantumHbar = quantumHbar;
        voiceDsp.lastQuantumTimestep = quantumTimestep;
      }

      if (voiceDsp.quantumCounter % 100) {
        for (auto& quantumOsc : voiceDsp.chordOscillators) {
          quantumOsc.applyPotential([this](float x) { return this->getPotential(x); }, 0.5f);
          quantumOsc.evolveWavefunction();
          quantumOsc.applyPotential([this](float x) { return this->getPotential(x); }, 0.5f);
        }
      }
    }
  }

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

  // Gain for sum of voice outputs with clamping to prevent extreme values
  float level = this->getRealFloatParam("level");
  float totalGain = 2.0f * level;

  totalOutput = totalOutput * ml::DSPVector(totalGain);
  
  // Debug: Check if we're storing data  
  static int debugStoreCounter = 0;
  if (debugStoreCounter++ % 1000 == 0) { // Every 1000 calls
    printf("ChordGenerator: level=%.6f, totalGain=%.6f, totalOutput[0-3]: %.6f %.6f %.6f %.6f\n", 
           level, totalGain, totalOutput[0], totalOutput[1], totalOutput[2], totalOutput[3]);
  }

  // publish totalOutput for oscilloscope widget
  DSPVectorArray<2> scopeOutput;
  scopeOutput.setRowVector<0>(totalOutput);  // Left channel
  scopeOutput.setRowVector<1>(totalOutput);  // Right channel (same as left for mono-to-stereo)
  this->storePublishedSignal("scope_output", scopeOutput, kFloatsPerDSPVector, 0);

  // Set outputs
  outputs[0] = totalOutput;
  outputs[1] = totalOutput;
  
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
    {"plaindefault", 0.9f},
    {"units", ""}
  }));

  // Inversion parameter - controls chord inversion and voicing (0-1)
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "inversion"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.5f},
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
    {"plaindefault", 0.0f},
    {"units", ""}
  }));

  // Quantum simulation parameters
  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "quantum_mass"},
    {"range", {0.1f, 2.0f}},
    {"plaindefault", 0.5f},
    {"units", ""}
  }));

  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "quantum_decoherence"},
    {"range", {0.0f, 0.2f}},
    {"plaindefault", 0.05f},
    {"units", ""}
  }));

  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "quantum_smoothing"},
    {"range", {0.0f, 1.0f}},
    {"plaindefault", 0.3f},
    {"units", ""}
  }));

  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "quantum_hbar"},
    {"range", {0.1f, 2.0f}},
    {"plaindefault", 1.0f},
    {"units", ""}
  }));

  params.push_back(std::make_unique<ml::ParameterDescription>(ml::WithValues{
    {"name", "quantum_timestep"},
    {"range", {0.001f, 0.1f}},
    {"plaindefault", 0.01f},
    {"units", ""}
  }));

  this->buildParams(params);
  this->setDefaultParams();
}

ml::DSPVector ChordGenerator::processVoice(int voiceIndex, ml::EventsToSignals::Voice& voice, ml::AudioContext* audioContext) {

  // safety checks
  if (voiceIndex < 0 || voiceIndex >= voiceDSP.size()) {
    return ml::DSPVector{0.0f};
  }
  if (!audioContext) {
    return ml::DSPVector{0.0f};
  }
  const float sr = audioContext->getSampleRate();
  if (sr <= 0.0f) {
    return ml::DSPVector{0.0f};
  }

  // Get voice control signals provided by EventsToSignals
  const ml::DSPVector vPitch = voice.outputs.row(ml::kPitch);
  const ml::DSPVector vGate = voice.outputs.row(ml::kGate);

  // Chord parameters are updated once per frame in processVector()

  // Convert MIDI pitch to Hz: 440 * 2^((note-69)/12)
  // Clamp pitch offset to prevent extreme frequency calculations that could generate NaN
  const ml::DSPVector vPitchOffset = clamp(vPitch - ml::DSPVector(69.0f), ml::DSPVector(-48.0f), ml::DSPVector(48.0f));
  const ml::DSPVector vPitchRatio = pow(ml::DSPVector(2.0f), vPitchOffset * ml::DSPVector(1.0f/12.0f));
  const ml::DSPVector vFreqHz = ml::DSPVector(440.0f) * vPitchRatio;

  float attack = this->getRealFloatParam("attack") * 1e-3f;
  float decay = 0.1f;     // 100ms decay
  float sustain = 1.0f;   // 100% sustain level
  float release = this->getRealFloatParam("release") * 1e-3f;

  // Only update ADSR coefficients if parameters changed (to avoid resetting envelope state)
  VoiceDSP& voiceDsp = voiceDSP[voiceIndex];
  if (attack != voiceDsp.lastAttack || release != voiceDsp.lastRelease) {
    voiceDsp.mADSR.coeffs = ml::ADSR::calcCoeffs(attack, decay, sustain, release, sr);
    voiceDsp.lastAttack = attack;
    voiceDsp.lastRelease = release;
  }

  // Get amplitude parameter with clamping to prevent extreme values
  float amplitude = this->getRealFloatParam("amplitude");

  // Get debug oscillator switch (0=Wavetable, 1=SineGen)
  float debugOsc = this->getRealFloatParam("debug_osc");
  bool useSineGen = debugOsc > 0.5f;  // Comparator at 0.5

  // Process ADSR envelope using gate signal scaled by amplitude
  // ml::ADSR expects gate+amp signal: the input value becomes both trigger and amplitude scaling
  const ml::DSPVector vGateWithAmp = vGate * ml::DSPVector(amplitude);
  const ml::DSPVector vEnvelope = voiceDSP[voiceIndex].mADSR(vGateWithAmp);

  // Generate full chord using the chord synthesis algorithm
  ml::DSPVector chordOutput{0.0f};

  for (int chordVoice = 0; chordVoice < kChordVoices; ++chordVoice) {
    // Get target amplitude from chord state and smooth it to prevent zippering
    float targetAmp = chordState.voiceAmplitudes[chordVoice];
    ml::DSPVector smoothedAmp = voiceDSP[voiceIndex].voiceAmpGlides[chordVoice](targetAmp);

    // Calculate frequency for this chord voice using the chord ratios
    const ml::DSPVector chordFreq = vFreqHz * ml::DSPVector(chordState.voiceRatios[chordVoice]);
    const ml::DSPVector vFreqNorm = chordFreq / ml::DSPVector(sr);

    // Get oscillator
    ml::DSPVector vOscillator;

    // ml::SineGen
    if (useSineGen) {
      vOscillator = voiceDSP[voiceIndex]
                    .chordSineGens[chordVoice](vFreqNorm);

    // wavefunction wavetable!
    } else if constexpr (kEnableQuantumSimulation) {
      vOscillator = voiceDSP[voiceIndex]
                    .chordOscillators[chordVoice]
                    .outputRealPart(vFreqNorm);

    }

    ml::DSPVector vOscWithAmp = vOscillator * smoothedAmp;
    const ml::DSPVector vOutput = vOscWithAmp * vEnvelope;
    chordOutput += vOutput;
  }

  return chordOutput;
}

// Quantum potential function implementations

float ChordGenerator::harmonicOscillatorPotential(float x) const {
  // Harmonic oscillator potential: V(x) = ½kx²
  // Center at x=0.5, spring constant k=10
  float x0 = x - 0.5f;  // Center at x=0.5
  float potential = 0.5f * 10.0f * x0 * x0;
  potential = std::clamp(potential, -10.0f, 10.0f);
  return potential;
}

float ChordGenerator::particleInBoxPotential(float x) const {
  // Particle in a box potential: V(x) = 0 inside box, ∞ outside
  // Box extends from (0.5 - width/2) to (0.5 + width/2)
  float boxHeight = 100.0;
  float boxCenter = 0.5f;
  float boxLeft = boxCenter - kBoxWidth * 0.5f;
  float boxRight = boxCenter + kBoxWidth * 0.5f;

  if (x >= boxLeft && x <= boxRight) {
    return 0.0f;  // Inside
  } else {
    return boxHeight;  // Outside
  }
}

float ChordGenerator::getPotential(float x) const {
  x = std::clamp(x, 0.0f, 1.0f);

  // Compile-time dispatch to selected potential function
  // TODO: realtime switch for potential functions
  if constexpr (kQuantumPotential == QuantumPotential::HARMONIC_OSCILLATOR) {
    return harmonicOscillatorPotential(x);
  } else if constexpr (kQuantumPotential == QuantumPotential::PARTICLE_IN_BOX) {
    return particleInBoxPotential(x);
  } else {
    return 0.0f;  // Default to free particle
  }
}
