#include "MicroSuono/nodes/GainNode.hpp"

namespace ms {

GainNode::GainNode(const std::string& id, float gain)
  : Node(id), gain_(gain) {
  addInputPort("in", PortType::Audio);
  addInputPort("gain", PortType::Control);
  addOutputPort("out", PortType::Audio);
  params.push_back({"gain", gain});
}

void GainNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioInputs || !audioInputs[0] || !audioOutputs || !audioOutputs[0]) return;

  for (int i = 0; i < nFrames; ++i) {
    audioOutputs[0][i] = audioInputs[0][i] * gain_;
  }
}

void GainNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs) {
  
  auto it = controlInputs.find("gain");
  if (it != controlInputs.end()) {
    if (std::holds_alternative<float>(it->second)) {
      setGain(std::get<float>(it->second));
    }
  }
}

void GainNode::setGain(float gain) {
  gain_ = gain;
  params[0].value = gain;
}

float GainNode::getGain() const {
  return gain_;
}

} // namespace ms
