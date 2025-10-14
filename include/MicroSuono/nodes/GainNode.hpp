#pragma once
#include "../Node.hpp"

namespace ms {

/** Gain/volume control node */
class GainNode : public Node {
public:
  GainNode(const std::string& id, float gain = 1.0f);
  
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  
  void setGain(float gain);
  float getGain() const;

private:
  float gain_;
};

} // namespace ms
