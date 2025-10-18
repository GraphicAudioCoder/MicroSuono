#include "MicroSuono/nodes/AudioInputNode.hpp"
#include <cstring>

namespace ms {

AudioInputNode::AudioInputNode(const std::string& id, int channelIndex)
  : Node(id), channelIndex_(channelIndex) {
  
  addOutputPort("out", PortType::Audio);
  params.push_back({"channel", static_cast<float>(channelIndex)});
}

void AudioInputNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
}

void AudioInputNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioOutputs || !audioOutputs[0]) return;
  
  // Get physical input from GraphManager
  const float* physicalInput = getPhysicalInput(channelIndex_);
  
  if (physicalInput) {
    // Copy physical input to output
    std::memcpy(audioOutputs[0], physicalInput, nFrames * sizeof(float));
  } else {
    // No input available, output silence
    std::memset(audioOutputs[0], 0, nFrames * sizeof(float));
  }
  
  // Apply fade-in envelope
  applyFadeIn(audioOutputs[0], nFrames);
}

} // namespace ms
