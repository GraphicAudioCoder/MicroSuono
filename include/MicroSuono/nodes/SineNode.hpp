#pragma once
#include "../Node.hpp"
#include <cmath>

namespace ms {

/** Sine wave oscillator node */
class SineNode : public Node {
public:
  SineNode(const std::string& id, float frequency = 440.0f);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  
  void setFrequency(float freq);

private:
  float frequency_;
  float phase_;
  float phaseIncrement_;
};

} // namespace ms
