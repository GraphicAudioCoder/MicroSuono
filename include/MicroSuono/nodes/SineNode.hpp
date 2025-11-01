#pragma once
#include "../Node.hpp"
#include <cmath>

namespace ms {

/** Sine wave oscillator node with amplitude and offset control */
class SineNode : public Node {
public:
  SineNode(const std::string& id, float frequency = 440.0f, float amplitude = 1.0f, float offset = 0.0f);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  
  void setFrequency(float freq);
  void setAmplitude(float amp);
  void setOffset(float off);

protected:
  float frequency_;        ///< Oscillator frequency in Hz
  float amplitude_;        ///< Output amplitude
  float offset_;           ///< DC offset
  float phase_;            ///< Current phase [0, 2π]
  float phaseIncrement_;   ///< Phase increment per sample
  
  // Smoothing for frequency changes to avoid phase discontinuities
  float targetFrequency_;  ///< Target frequency for smoothing
  float currentFrequency_; ///< Current smoothed frequency
  float frequencyDelta_;   ///< Per-sample frequency delta
  int smoothingSamples_;   ///< Number of samples for smoothing
};

} // namespace ms
