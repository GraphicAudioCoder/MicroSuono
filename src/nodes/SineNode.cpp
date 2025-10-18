#include "MicroSuono/nodes/SineNode.hpp"

namespace ms {

SineNode::SineNode(const std::string& id, float frequency, float amplitude, float offset)
  : Node(id), 
    frequency_(frequency), 
    amplitude_(amplitude),
    offset_(offset),
    phase_(0.0f), 
    phaseIncrement_(0.0f) {
  addInputPort("frequency", PortType::Control);
  addInputPort("amplitude", PortType::Control);
  addInputPort("offset", PortType::Control);
  addOutputPort("out", PortType::Audio);
  params.push_back({"frequency", frequency});
  params.push_back({"amplitude", amplitude});
  params.push_back({"offset", offset});
}

void SineNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  phase_ = 0.0f;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate);
}

void SineNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioOutputs || !audioOutputs[0]) return;

  for (int i = 0; i < nFrames; ++i) {
    // Generate sine: offset + amplitude * sin(phase)
    audioOutputs[0][i] = offset_ + amplitude_ * std::sin(phase_);
    phase_ += phaseIncrement_;
    
    if (phase_ >= 2.0f * M_PI) {
      phase_ -= 2.0f * M_PI;
    }
  }
  
  // Apply fade-in envelope
  applyFadeIn(audioOutputs[0], nFrames);
}

void SineNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs) {
  
  auto freqIt = controlInputs.find("frequency");
  if (freqIt != controlInputs.end() && std::holds_alternative<float>(freqIt->second)) {
    setFrequency(std::get<float>(freqIt->second));
  }
  
  auto ampIt = controlInputs.find("amplitude");
  if (ampIt != controlInputs.end() && std::holds_alternative<float>(ampIt->second)) {
    setAmplitude(std::get<float>(ampIt->second));
  }
  
  auto offsetIt = controlInputs.find("offset");
  if (offsetIt != controlInputs.end() && std::holds_alternative<float>(offsetIt->second)) {
    setOffset(std::get<float>(offsetIt->second));
  }
}

void SineNode::setFrequency(float freq) {
  frequency_ = freq;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate_);
  params[0].value = freq;
}

void SineNode::setAmplitude(float amp) {
  amplitude_ = amp;
  params[1].value = amp;
}

void SineNode::setOffset(float off) {
  offset_ = off;
  params[2].value = off;
}

} // namespace ms
