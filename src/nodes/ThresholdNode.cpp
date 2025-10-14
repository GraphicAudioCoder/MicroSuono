#include "MicroSuono/nodes/ThresholdNode.hpp"
#include <cmath>
#include <iostream>

namespace ms {

ThresholdNode::ThresholdNode(const std::string& id, float threshold)
  : Node(id), threshold_(threshold), wasAboveThreshold_(false) {
  
  // Input: audio signal and threshold control
  addInputPort("in", PortType::Audio);
  addInputPort("threshold", PortType::Control);
  
  // Output: passthrough audio and trigger event
  addOutputPort("out", PortType::Audio);
  addOutputPort("trigger", PortType::Event);
}

void ThresholdNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs) {
  
  // Update threshold from control input
  auto it = controlInputs.find("threshold");
  if (it != controlInputs.end()) {
    if (std::holds_alternative<float>(it->second)) {
      threshold_ = std::get<float>(it->second);
    }
  }
}

void ThresholdNode::processEvents(
  const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
  std::unordered_map<std::string, std::vector<Event>>& eventOutputs) {
  
  // Events will be generated during audio processing
  // Initialize empty output queue
  eventOutputs["trigger"] = std::vector<Event>();
}

void ThresholdNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (!audioInputs || !audioInputs[0] || !audioOutputs || !audioOutputs[0]) {
    return;
  }
  
  const float* in = audioInputs[0];
  float* out = audioOutputs[0];
  
  // Process audio and detect threshold crossings
  for (int i = 0; i < nFrames; ++i) {
    // Passthrough audio
    out[i] = in[i];
    
    // Check for rising edge (crosses threshold upward)
    bool isAboveThreshold = in[i] > threshold_;
    
    if (isAboveThreshold && !wasAboveThreshold_) {
      // Rising edge detected - generate trigger event
      std::cout << "ThresholdNode: Trigger at sample " << i 
                << " (value: " << in[i] << ", threshold: " << threshold_ << ")" << std::endl;
      
      // Note: Events generated here would need to be stored and
      // added to eventOutputs in the next processEvents() call
      // For now, we just log them
    }
    
    wasAboveThreshold_ = isAboveThreshold;
  }
}

void ThresholdNode::setThreshold(float threshold) {
  threshold_ = threshold;
}

float ThresholdNode::getThreshold() const {
  return threshold_;
}

} // namespace ms
