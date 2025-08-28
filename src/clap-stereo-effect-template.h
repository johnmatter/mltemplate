#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class

#ifdef HAS_GUI
class ClapStereoEffectTemplateGUI;
#endif

class ClapStereoEffectTemplate : public ml::CLAPSignalProcessor<> {
private:
  // the AudioContext's EventsToSignals handles state
  ml::AudioContext* audioContext = nullptr;  // Set by wrapper

  // Stereo effect processing state
  struct EffectState {
    // Add your effect-specific state here
    float leftGain = 1.0f;
    float rightGain = 1.0f;
  };
  EffectState effectState;

  // Track if effect is active for CLAP sleep/continue
  bool isActive = false;

public:
  ClapStereoEffectTemplate();
  ~ClapStereoEffectTemplate() = default;

  // SignalProcessor interface
  void setSampleRate(double sr);
  void buildParameterDescriptions();

  void processAudioContext();
  void setAudioContext(ml::AudioContext* ctx) { audioContext = ctx; }

  // Effect activity for CLAP sleep/continue
  bool hasActiveVoices() const override { return isActive; }

  // Plugin-specific interface
  const ml::ParameterTree& getParameterTree() const { return this->_params; }

private:
  // Helper methods for effect processing
  void processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel);
  void updateEffectState();
};

