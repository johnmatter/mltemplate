#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class

#ifdef HAS_GUI
class ClapStereoEffectTemplateGUI;
#endif

class ClapStereoEffectTemplate : public ml::CLAPSignalProcessor<> {
private:
  // the AudioContext's EventsToSignals handles state
  ml::AudioContext* audioContext = nullptr;  // Set by wrapper

  // TapeHack saturation processing state
  struct EffectState {
    ml::DSPVector fpdL;
    ml::DSPVector fpdR;
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
  
  // TapeHack saturation algorithm
  ml::DSPVector processTapeHackSaturation(const ml::DSPVector& inputSamples, float inputGain, float outputGain, ml::DSPVector& fpd);
};

