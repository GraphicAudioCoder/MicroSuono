#include "MicroSuono/nodes/SineOscillatorNode.hpp"
#include <cmath>

namespace ms {

SineOscillatorNode::SineOscillatorNode(const std::string& id, 
                                       float defaultFrequency, 
                                       float defaultAmplitude)
  : Node(id)
  , defaultFrequency_(defaultFrequency)
  , defaultAmplitude_(defaultAmplitude)
  , currentFrequency_(defaultFrequency)
  , currentAmplitude_(defaultAmplitude)
  , phase_(0.0f)
  , phaseIncrement_(0.0f)
  , targetFrequency_(defaultFrequency)
  , targetAmplitude_(defaultAmplitude)
  , frequencySmoothingCoeff_(0.0f)
  , amplitudeSmoothingCoeff_(0.0f)
{
  // Add control input ports
  addInputPort("frequency", PortType::Control);
  addInputPort("amplitude", PortType::Control);
  
  // Add audio output port
  addOutputPort("audio_out", PortType::Audio);
}

void SineOscillatorNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  
  // Calculate smoothing coefficients for parameter changes
  // tau = smoothing time constant, fs = sample rate
  // coefficient = exp(-1 / (tau * fs))
  // This creates an exponential smoothing that prevents clicks/pops
  float tau = SMOOTHING_TIME_MS / 1000.0f; // Convert to seconds
  frequencySmoothingCoeff_ = std::exp(-1.0f / (tau * sampleRate));
  amplitudeSmoothingCoeff_ = std::exp(-1.0f / (tau * sampleRate));
  
  updatePhaseIncrement();
}

void SineOscillatorNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs)
{
  // Read frequency control input if connected, otherwise use default
  if (controlInputs.find("frequency") != controlInputs.end()) {
    const auto& freqValue = controlInputs.at("frequency");
    if (std::holds_alternative<float>(freqValue)) {
      targetFrequency_ = std::get<float>(freqValue);
    }
  } else {
    targetFrequency_ = defaultFrequency_;
  }
  
  // Read amplitude control input if connected, otherwise use default
  if (controlInputs.find("amplitude") != controlInputs.end()) {
    const auto& ampValue = controlInputs.at("amplitude");
    if (std::holds_alternative<float>(ampValue)) {
      targetAmplitude_ = std::get<float>(ampValue);
    }
  } else {
    targetAmplitude_ = defaultAmplitude_;
  }
}

void SineOscillatorNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  float* output = audioOutputs[0];
  
  for (int i = 0; i < nFrames; ++i) {
    // Smooth frequency changes to avoid clicks/phase discontinuities
    currentFrequency_ = targetFrequency_ + frequencySmoothingCoeff_ * (currentFrequency_ - targetFrequency_);
    updatePhaseIncrement();
    
    // Smooth amplitude changes to avoid clicks/pops
    currentAmplitude_ = targetAmplitude_ + amplitudeSmoothingCoeff_ * (currentAmplitude_ - targetAmplitude_);
    
    // Generate sine wave with smoothed amplitude
    output[i] = std::sin(phase_) * currentAmplitude_;
    
    // Update phase
    phase_ += phaseIncrement_;
    
    // Wrap phase to [0, 2π]
    if (phase_ >= 2.0f * M_PI) {
      phase_ -= 2.0f * M_PI;
    }
  }
  
  // Apply fade-in if enabled
  applyFadeIn(output, nFrames);
}

void SineOscillatorNode::updatePhaseIncrement() {
  phaseIncrement_ = 2.0f * M_PI * currentFrequency_ / sampleRate_;
}

} // namespace ms
