#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class

#ifdef HAS_GUI
class TanhSaturatorGUI;
#endif

class TanhSaturator : public ml::CLAPSignalProcessor<> {
private:
  // the AudioContext's EventsToSignals handles state
  ml::AudioContext* audioContext = nullptr;  // Set by wrapper

  // EffectState holds a per-instance processing state for the effect.
  // For simple stateless effects like tanh saturation, this struct may remain empty.
  // More complex effects (such as loopers, delays, or reverbs) can store their
  // internal DSP state here. For example:
  //   - A delay or looper effect might keep a circular buffer of samples using ml::DSPVectorArray,
  //     to store audio history for feedback or looping.
  //   - Effects with modulation or envelopes might store LFO phase, envelope state, or filter coefficients.
  //   - Any state that must persist across process calls but is not global to the plugin
  //     (such as per-voice or per-channel buffers) should be kept here.
  // Data that should NOT be kept here:
  //   - Global plugin state, shared resources, or static configuration that does not change per instance.
  //   - GUI state, pointers to the audio context, or references to external systems.
  //   - Large static tables or resources that can be shared across instances (these should be static or global).
  struct EffectState {
    // Lowpass filters for left and right channels
    // OnePole filters following aaltoverb pattern
    ml::OnePole lowpassL;
    ml::OnePole lowpassR;
  };
  EffectState effectState;

  // Track if effect is active for CLAP sleep/continue
  bool isActive = false;

public:
  TanhSaturator();
  ~TanhSaturator() = default;

  // SignalProcessor interface  
  void setSampleRate(double sr) override;
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
  
  // Tanh saturation algorithm
  ml::DSPVector processTanhSaturation(const ml::DSPVector& inputSamples, float inputGain, float outputGain);
};
