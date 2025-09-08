#pragma once

#include "CLAPExport.h"  // Includes madronalib core + CLAPSignalProcessor base class
#include <vector>
#include <mutex>

#ifdef HAS_GUI
class OscilloscopeGUI;
class OscilloscopeWidget;
#endif

class Oscilloscope : public ml::CLAPSignalProcessor<> {
private:
  // Set by wrapper
  ml::AudioContext* audioContext = nullptr;

  struct EffectState {
  };
  EffectState effectState;

  // Track if effect is active for CLAP sleep/continue
  bool isActive = false;

  // Oscilloscope buffer
  static constexpr size_t kOscilloscopeBufferSize = 128; // 2 DSPVectors worth of samples
  std::vector<float> oscilloscopeBuffer;
  std::mutex oscilloscopeMutex;
  size_t oscilloscopeWriteIndex = 0;

public:
  Oscilloscope();
  ~Oscilloscope() = default;

  // SignalProcessor interface
  void setSampleRate(double sr);
  void buildParameterDescriptions();

  void processAudioContext();
  void setAudioContext(ml::AudioContext* ctx) { audioContext = ctx; }

  // Effect activity for CLAP sleep/continue
  bool hasActiveVoices() const override { return isActive; }

  // Plugin-specific interface
  const ml::ParameterTree& getParameterTree() const { return this->_params; }

  // Oscilloscope interface
  std::vector<float> getOscilloscopeData() const;

private:
  // Helper methods for oscilloscope processing
  void processStereoEffect(ml::DSPVector& leftChannel, ml::DSPVector& rightChannel);
  void updateEffectState();
};

