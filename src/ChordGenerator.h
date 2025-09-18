#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class

#ifdef HAS_GUI
class ChordGeneratorGUI;
#endif

class ChordGenerator : public ml::CLAPSignalProcessor<> {
private:
  // the AudioContext's EventsToSignals handles state
  ml::AudioContext* audioContext = nullptr;  // Set by wrapper

  // Per-voice DSP components (simplified approach like demo)
  struct VoiceDSP {
    ml::SawGen chordOscillators[5];  // 5 oscillators per voice for chord
    ml::ADSR mADSR;

    // These are initialized properly in the constructor using parameter defaults
    float lastAttack = -1.0f;
    float lastRelease = -1.0f;
  };
  std::array<VoiceDSP, 16> voiceDSP;

  // Simple voice activity tracking for CLAP
  int activeVoiceCount = 0;

  // Chord definitions (from Plaits)
  static constexpr int kNumChords = 11;
  static constexpr int kNotesPerChord = 4;
  static constexpr int kNumVoices = kNotesPerChord + 1;  // 5 voices like Plaits

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

  // Simplified state for chord synthesis
  struct ChordState {
    // Current chord selection and inversion
    int currentChord = 0;
    float currentInversion = 0.0f;

    // Base chord ratios (from chord bank with detuning, before inversion)
    float baseChordRatios[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    // Voice ratios and amplitudes (computed from chord + inversion)
    float voiceRatios[5] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float voiceAmplitudes[5] = {0.25f, 0.25f, 0.25f, 0.25f, 0.25f};
  };
  ChordState chordState;

public:
  ChordGenerator();
  ~ChordGenerator() = default;

  // SignalProcessor interface
  void setSampleRate(double sr) override;
  void buildParameterDescriptions();

  void processVector(const ml::DSPVectorDynamic& inputs, ml::DSPVectorDynamic& outputs, void* stateData = nullptr) override;

  // Synth activity for CLAP sleep/continue
  bool hasActiveVoices() const override { return activeVoiceCount > 0; }

  // Plugin-specific interface
  const ml::ParameterTree& getParameterTree() const { return this->_params; }

private:

  // Chord stuff from Plaits
  void selectChord(float harmonicsParam, float detuneCents);
  void computeChordInversion(float inversionParam);
  float semitonesToRatio(float semitones) { return std::pow(2.0f, semitones / 12.0f); }

  // Voice processing
  ml::DSPVector processVoice(int voiceIndex, ml::EventsToSignals::Voice& voice, ml::AudioContext* audioContext);
};
