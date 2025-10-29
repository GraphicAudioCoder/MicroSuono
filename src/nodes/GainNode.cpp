#include "MicroSuono/nodes/GainNode.hpp"

namespace ms {

GainNode::GainNode(const std::string& id, float gain)
  : Node(id), 
    targetGain_(gain),
    currentGain_(gain),
    deltaGain_(0.0f),
    needsSmoothing_(false),
    samplesPerBlock_(512) {
  addInputPort("in", PortType::Audio);
  addInputPort("gain", PortType::Control);
  addOutputPort("out", PortType::Audio);
  params.push_back({"gain", gain});
}

void GainNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  samplesPerBlock_ = blockSize;
}

void GainNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioOutputs || !audioOutputs[0]) return;

  // If no valid input, output silence
  if (!audioInputs || !audioInputs[0]) {
    for (int i = 0; i < nFrames; ++i) {
      audioOutputs[0][i] = 0.0f;
    }
    return;
  }

  // Check if we have audio-rate modulation (second audio input from audio->control connection)
  if (audioInputs[1] != nullptr) {
    // Audio-rate modulation: use modulation signal sample-by-sample
    for (int i = 0; i < nFrames; ++i) {
      audioOutputs[0][i] = audioInputs[0][i] * audioInputs[1][i];
    }
  } else {
    // Control-rate: apply JSFX-style smoothing
    if (needsSmoothing_) {
      deltaGain_ = (targetGain_ - currentGain_) / samplesPerBlock_;
      needsSmoothing_ = false;
    }
    
    for (int i = 0; i < nFrames; ++i) {
      audioOutputs[0][i] = audioInputs[0][i] * currentGain_;
      currentGain_ += deltaGain_;
    }
    
    // Snap to target at end of block to avoid drift
    if (nFrames == samplesPerBlock_) {
      currentGain_ = targetGain_;
      deltaGain_ = 0.0f;
    }
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
  targetGain_ = gain;
  needsSmoothing_ = true;
  params[0].value = gain;
}

float GainNode::getGain() const {
  return targetGain_;
}

} // namespace ms
