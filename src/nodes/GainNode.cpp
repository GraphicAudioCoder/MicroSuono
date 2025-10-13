#include "MicroSuono/nodes/GainNode.hpp"

namespace ms {

GainNode::GainNode(const std::string& id, float gain)
  : Node(id, 1, 1), gain_(gain) {
  params.push_back({"gain", gain});
}

void GainNode::process(const float* const* inputs, float** outputs, int nFrames) {
  if (!inputs || !inputs[0] || !outputs || !outputs[0]) return;

  for (int i = 0; i < nFrames; ++i) {
    outputs[0][i] = inputs[0][i] * gain_;
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
