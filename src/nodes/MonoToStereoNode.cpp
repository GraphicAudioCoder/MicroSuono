#include "MicroSuono/nodes/MonoToStereoNode.hpp"
#include <cmath>

namespace ms {

MonoToStereoNode::MonoToStereoNode(const std::string& id, float pan)
  : Node(id), pan_(pan) {
  
  // Input ports
  addInputPort("in", PortType::Audio);
  addInputPort("pan", PortType::Control);
  
  // Output ports
  addOutputPort("left", PortType::Audio);
  addOutputPort("right", PortType::Audio);
  
  getParams().push_back({"pan", pan});
}

void MonoToStereoNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
}

void MonoToStereoNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioInputs || !audioInputs[0] || !audioOutputs || !audioOutputs[0] || !audioOutputs[1]) return;
  
  const float* input = audioInputs[0];
  float* left = audioOutputs[0];
  float* right = audioOutputs[1];
  
  // Calculate stereo gains using constant power panning
  // This maintains equal loudness across the stereo field
  float angle = (pan_ + 1.0f) * 0.25f * M_PI; // Map -1..+1 to 0..π/2
  float leftGain = std::cos(angle);
  float rightGain = std::sin(angle);
  
  // Apply panning
  for (int i = 0; i < nFrames; ++i) {
    left[i] = input[i] * leftGain;
    right[i] = input[i] * rightGain;
  }
}

void MonoToStereoNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs) {
  
  auto it = controlInputs.find("pan");
  if (it != controlInputs.end()) {
    if (std::holds_alternative<float>(it->second)) {
      pan_ = std::get<float>(it->second);
      
      // Clamp pan to valid range
      if (pan_ < -1.0f) pan_ = -1.0f;
      if (pan_ > 1.0f) pan_ = 1.0f;
      
      setParam("pan", pan_);
    }
  }
}

} // namespace ms
