#include <nodes/MixerNode.hpp>
#include <algorithm>
#include <cmath>

namespace ms {

MixerNode::MixerNode(const std::string& id, int numInputs, bool stereo)
    : Node(id), numInputs_(numInputs), stereo_(stereo),
      masterGain_(1.0f), targetMasterGain_(1.0f), 
      currentMasterGain_(1.0f), masterGainDelta_(0.0f),
      smoothingSamples_(256) {
  
  // Create input ports
  for (int i = 0; i < numInputs_; ++i) {
    addInputPort("in_" + std::to_string(i), PortType::Audio);
  }
  
  // Create output port(s)
  if (stereo_) {
    addOutputPort("out_left", PortType::Audio);
    addOutputPort("out_right", PortType::Audio);
  } else {
    addOutputPort("out", PortType::Audio);
  }
  
  // Initialize gain arrays
  channelGains_.resize(numInputs_, 1.0f);
  targetGains_.resize(numInputs_, 1.0f);
  currentGains_.resize(numInputs_, 1.0f);
  gainDeltas_.resize(numInputs_, 0.0f);
  
  // Initialize pan arrays (0.0 = center by default)
  channelPans_.resize(numInputs_, 0.0f);
  targetPans_.resize(numInputs_, 0.0f);
  currentPans_.resize(numInputs_, 0.0f);
  panDeltas_.resize(numInputs_, 0.0f);
}

void MixerNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
  
  // Set smoothing time (e.g., 5ms)
  smoothingSamples_ = static_cast<int>((5.0f / 1000.0f) * sampleRate);
  if (smoothingSamples_ < 1) smoothingSamples_ = 1;
  
  updateGainSmoothing();
}

void MixerNode::setChannelGain(int channel, float gain) {
  if (channel >= 0 && channel < numInputs_) {
    targetGains_[channel] = std::max(0.0f, gain);
    updateGainSmoothing();
  }
}

void MixerNode::setChannelPan(int channel, float pan) {
  if (channel >= 0 && channel < numInputs_) {
    // Clamp pan to [-1.0, 1.0]
    targetPans_[channel] = std::max(-1.0f, std::min(1.0f, pan));
    updatePanSmoothing();
  }
}

void MixerNode::setMasterGain(float gain) {
  targetMasterGain_ = std::max(0.0f, gain);
  updateGainSmoothing();
}

void MixerNode::updateGainSmoothing() {
  // Calculate deltas for smooth transitions
  for (int i = 0; i < numInputs_; ++i) {
    gainDeltas_[i] = (targetGains_[i] - currentGains_[i]) / smoothingSamples_;
  }
  masterGainDelta_ = (targetMasterGain_ - currentMasterGain_) / smoothingSamples_;
}

void MixerNode::updatePanSmoothing() {
  // Calculate deltas for smooth pan transitions
  for (int i = 0; i < numInputs_; ++i) {
    panDeltas_[i] = (targetPans_[i] - currentPans_[i]) / smoothingSamples_;
  }
}

void MixerNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  if (stereo_) {
    // Stereo mixing with proper panning
    float* outLeft = audioOutputs[0];
    float* outRight = audioOutputs[1];
    
    // Clear output buffers
    std::fill(outLeft, outLeft + nFrames, 0.0f);
    std::fill(outRight, outRight + nFrames, 0.0f);
    
    // Mix all inputs with panning
    for (int ch = 0; ch < numInputs_; ++ch) {
      if (!audioInputs[ch]) continue;
      
      const float* input = audioInputs[ch];
      float gain = currentGains_[ch];
      float delta = gainDeltas_[ch];
      float pan = currentPans_[ch];
      float panDelta = panDeltas_[ch];
      
      for (int i = 0; i < nFrames; ++i) {
        // Constant power panning law
        // pan: -1.0 (full left) to 1.0 (full right), 0.0 = center
        float panAngle = (pan + 1.0f) * 0.25f * 3.14159265359f; // Map to [0, π/2]
        float leftGain = std::cos(panAngle);   // Full at left, 0.707 at center
        float rightGain = std::sin(panAngle);  // 0.707 at center, full at right
        
        float sample = input[i] * gain;
        outLeft[i] += sample * leftGain;
        outRight[i] += sample * rightGain;
        
        // Update gain smoothing
        if (std::abs(gain - targetGains_[ch]) > 0.0001f) {
          gain += delta;
        }
        
        // Update pan smoothing
        if (std::abs(pan - targetPans_[ch]) > 0.0001f) {
          pan += panDelta;
        }
      }
      
      currentGains_[ch] = gain;
      currentPans_[ch] = pan;
    }
    
    // Apply master gain with smoothing
    float masterGain = currentMasterGain_;
    for (int i = 0; i < nFrames; ++i) {
      outLeft[i] *= masterGain;
      outRight[i] *= masterGain;
      
      if (std::abs(masterGain - targetMasterGain_) > 0.0001f) {
        masterGain += masterGainDelta_;
      }
    }
    currentMasterGain_ = masterGain;
    
    // Apply fade-in
    applyFadeIn(outLeft, nFrames);
    applyFadeIn(outRight, nFrames);
    
  } else {
    // Mono mixing
    float* output = audioOutputs[0];
    std::fill(output, output + nFrames, 0.0f);
    
    // Mix all inputs
    for (int ch = 0; ch < numInputs_; ++ch) {
      if (!audioInputs[ch]) continue;
      
      const float* input = audioInputs[ch];
      float gain = currentGains_[ch];
      float delta = gainDeltas_[ch];
      
      for (int i = 0; i < nFrames; ++i) {
        output[i] += input[i] * gain;
        
        // Update gain smoothing
        if (std::abs(gain - targetGains_[ch]) > 0.0001f) {
          gain += delta;
        }
      }
      
      currentGains_[ch] = gain;
    }
    
    // Apply master gain with smoothing
    float masterGain = currentMasterGain_;
    for (int i = 0; i < nFrames; ++i) {
      output[i] *= masterGain;
      
      if (std::abs(masterGain - targetMasterGain_) > 0.0001f) {
        masterGain += masterGainDelta_;
      }
    }
    currentMasterGain_ = masterGain;
    
    // Apply fade-in
    applyFadeIn(output, nFrames);
  }
}

} // namespace ms
