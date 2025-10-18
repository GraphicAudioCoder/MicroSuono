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

private:
  float frequency_;
  float amplitude_;
  float offset_;
  float phase_;
  float phaseIncrement_;
};

} // namespace ms
