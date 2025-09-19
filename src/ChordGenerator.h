#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class
#include "dsp/MLDSPWavetableGen.h"
#include "MLDSPGens.h"  // For ml::SineGen debug oscillator
#include "MLDSPFilters.h"  // For ml::DCBlocker

#ifdef HAS_GUI
class ChordGeneratorGUI;
#endif

class ChordGenerator : public ml::CLAPSignalProcessor<ml::SignalProcessor> {
public:
  // Voice configuration - accessible by CLAP wrapper for polyphony reporting
  static constexpr int kNumVoices = 1;  // Monophonic for now - focus on Schrödinger equation

  // Chord definitions (from Plaits) - declared early so they can be used in struct definitions
  static constexpr int kNumChords = 11;
  static constexpr int kNotesPerChord = 4;
  static constexpr int kChordVoices = 5;  // Number of oscillators per chord (distinct from polyphonic voices)

private:
  // the AudioContext's EventsToSignals handles state
  ml::AudioContext* audioContext = nullptr;  // Set by wrapper

  // Per-voice DSP components
  struct VoiceDSP {
    // kChordVoices oscillators per voice for chord.
    // We hear three, with three faded in and out by the "inversion" parameter
    ml::WavetableGen chordOscillators[kChordVoices];
    ml::SineGen chordSineGens[kChordVoices];  // Debug alternative to wavetable
    ml::ADSR mADSR;

    // Smoothed voice amplitudes to prevent zippering during inversion changes (following sumu pattern)
    ml::LinearGlide voiceAmpGlides[kChordVoices];
    
    // DC blockers for each chord voice to prevent DC buildup during envelope transitions
    ml::DCBlocker voiceDCBlockers[kChordVoices];

    // These are initialized properly in the constructor using parameter defaults
    float lastAttack = -1.0f;
    float lastRelease = -1.0f;
    
    // Track oscillator type changes per voice for DC blocker reset
    float lastDebugOsc = -1.0f;
  };
  
  std::array<VoiceDSP, kNumVoices> voiceDSP;

  // Chord bank: semitone offsets from root note
  static constexpr float chords_[kNumChords][kNotesPerChord] = {
    { 0.0f,  0.01f, 11.99f, 12.0f },  // OCT: Octave
    { 0.0f,  7.0f,   7.01f, 12.0f },  // 5: Fifth
    { 0.0f,  5.0f,   7.0f,  12.0f },  // sus4: Suspended 4th
    { 0.0f,  3.0f,   7.0f,  12.0f },  // m: Minor
    { 0.0f,  3.0f,   7.0f,  10.0f },  // m7: Minor 7th
    { 0.0f,  3.0f,  10.0f,  14.0f },  // m9: Minor 9th
    { 0.0f,  3.0f,  10.0f,  17.0f },  // m11: Minor 11th
    { 0.0f,  2.0f,   9.0f,  16.0f },  // 69: 6/9 chord
    { 0.0f,  4.0f,  11.0f,  14.0f },  // M9: Major 9th
    { 0.0f,  4.0f,   7.0f,  11.0f },  // M7: Major 7th
    { 0.0f,  4.0f,   7.0f,  12.0f },  // M: Major
  };

  // State for chord synthesis
  struct ChordState {
    // Current chord selection and inversion
    int currentChord = 0;
    float currentInversion = 0.0f;

    // Base chord ratios (from chord bank with detuning, before inversion)
    float baseChordRatios[kNotesPerChord] = {1.0f, 1.0f, 1.0f, 1.0f};

    // Voice ratios and amplitudes (computed from chord + inversion)
    float voiceRatios[kChordVoices] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float voiceAmplitudes[kChordVoices] = {0.25f, 0.25f, 0.25f, 0.25f, 0.25f};
  };
  ChordState chordState;

  // DC blocker to remove DC offset from summed oscillator output
  ml::DCBlocker dcBlocker;

public:
  ChordGenerator();
  ~ChordGenerator() = default;

  // SignalProcessor interface
  void setSampleRate(double sr) override;
  void buildParameterDescriptions();

  void processVector(const ml::DSPVectorDynamic& inputs, ml::DSPVectorDynamic& outputs, void* stateData = nullptr) override;

  // Synth activity for CLAP sleep/continue - always return true for simplicity
  bool hasActiveVoices() const override { return true; }

  // Plugin-specific interface
  const ml::ParameterTree& getParameterTree() const { return this->_params; }

  // Voice processing
  ml::DSPVector processVoice(int voiceIndex, ml::EventsToSignals::Voice& voice, ml::AudioContext* audioContext);

private:

  // Chord stuff from Plaits
  void selectChord(float harmonicsParam, float detuneCents);
  void computeChordInversion(float inversionParam);
  float semitonesToRatio(float semitones) { return std::pow(2.0f, semitones / 12.0f); }
};
