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

protected:
  int numInputs_;                          ///< Number of input channels
  bool stereo_;                            ///< Stereo output mode
  
  // Per-channel gains with smoothing
  std::vector<float> channelGains_;        ///< Target gains per channel
  std::vector<float> targetGains_;         ///< Target gains for smoothing
  std::vector<float> currentGains_;        ///< Current smoothed gains
  std::vector<float> gainDeltas_;          ///< Per-sample gain deltas
  
  // Per-channel panning (-1.0 to 1.0, only for stereo)
  std::vector<float> channelPans_;         ///< Target pans per channel
  std::vector<float> targetPans_;          ///< Target pans for smoothing
  std::vector<float> currentPans_;         ///< Current smoothed pans
  std::vector<float> panDeltas_;           ///< Per-sample pan deltas
  
  float masterGain_;                       ///< Master output gain
  float targetMasterGain_;                 ///< Target master gain
  float currentMasterGain_;                ///< Current smoothed master gain
  float masterGainDelta_;                  ///< Per-sample master gain delta
  
  int smoothingSamples_;                   ///< Number of samples for smoothing
  
  void updateGainSmoothing();
  void updatePanSmoothing();
};

} // namespace ms
