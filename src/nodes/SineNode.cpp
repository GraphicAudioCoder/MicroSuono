#include "MicroSuono/nodes/SineNode.hpp"

namespace ms {

SineNode::SineNode(const std::string& id, float frequency)
  : Node(id), frequency_(frequency), phase_(0.0f), phaseIncrement_(0.0f) {
  addInputPort("frequency", PortType::Control);
  addOutputPort("out", PortType::Audio);
  params.push_back({"frequency", frequency});
}

void SineNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  phase_ = 0.0f;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate);
}

void SineNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioOutputs || !audioOutputs[0]) return;

  for (int i = 0; i < nFrames; ++i) {
    audioOutputs[0][i] = std::sin(phase_);
    phase_ += phaseIncrement_;
    
    if (phase_ >= 2.0f * M_PI) {
      phase_ -= 2.0f * M_PI;
    }
  }
}

void SineNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs) {
  
  auto it = controlInputs.find("frequency");
  if (it != controlInputs.end()) {
    if (std::holds_alternative<float>(it->second)) {
      setFrequency(std::get<float>(it->second));
    }
  }
}

void SineNode::setFrequency(float freq) {
  frequency_ = freq;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate_);
  params[0].value = freq;
}

} // namespace ms
