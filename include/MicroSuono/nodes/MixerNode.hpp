#pragma once
#include <MicroSuono/Node.hpp>
#include <vector>

namespace ms {

/**
 * Mixer Node - Mixes N audio inputs into one or two outputs
 * Demonstrates use of vector inputs and per-channel gain control
 */
class MixerNode : public Node {
public:
  MixerNode(const std::string& id, int numInputs = 4, bool stereo = false);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  
  /** Set gain for a specific input channel (0.0 to 1.0+) */
  void setChannelGain(int channel, float gain);
  
  /** Set pan for a specific input channel (-1.0 = full left, 0.0 = center, 1.0 = full right) 
   *  Only effective when stereo output is enabled */
  void setChannelPan(int channel, float pan);
  
  /** Set master output gain */
  void setMasterGain(float gain);
  
  /** Get number of input channels */
  int getNumInputs() const { return numInputs_; }
  
  /** Is stereo output? */
  bool isStereo() const { return stereo_; }

private:
  int numInputs_;
  bool stereo_;
  
  // Per-channel gains with smoothing
  std::vector<float> channelGains_;
  std::vector<float> targetGains_;
  std::vector<float> currentGains_;
  std::vector<float> gainDeltas_;
  
  // Per-channel panning (-1.0 to 1.0, only for stereo)
  std::vector<float> channelPans_;
  std::vector<float> targetPans_;
  std::vector<float> currentPans_;
  std::vector<float> panDeltas_;
  
  float masterGain_;
  float targetMasterGain_;
  float currentMasterGain_;
  float masterGainDelta_;
  
  int smoothingSamples_;
  
  void updateGainSmoothing();
  void updatePanSmoothing();
};

} // namespace ms
