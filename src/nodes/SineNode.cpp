#include "MicroSuono/nodes/SineNode.hpp"

namespace ms {

SineNode::SineNode(const std::string& id, float frequency)
  : Node(id, 0, 1), frequency_(frequency), phase_(0.0f), phaseIncrement_(0.0f) {
  params.push_back({"frequency", frequency});
}

void SineNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  phase_ = 0.0f;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate);
}

void SineNode::process(const float* const* inputs, float** outputs, int nFrames) {
  if (!outputs || !outputs[0]) return;

  for (int i = 0; i < nFrames; ++i) {
    outputs[0][i] = std::sin(phase_);
    phase_ += phaseIncrement_;
    
    if (phase_ >= 2.0f * M_PI) {
      phase_ -= 2.0f * M_PI;
    }
  }
}

void SineNode::setFrequency(float freq) {
  frequency_ = freq;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate_);
  params[0].value = freq;
}

} // namespace ms
