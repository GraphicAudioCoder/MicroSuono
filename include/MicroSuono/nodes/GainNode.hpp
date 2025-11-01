#pragma once
#include "../Node.hpp"

namespace ms {

/** Gain/volume control node with smooth parameter transitions */
class GainNode : public Node {
public:
  GainNode(const std::string& id, float gain = 1.0f);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  
  void setGain(float gain);
  float getGain() const;

protected:
  float targetGain_;      ///< Target gain value
  float currentGain_;     ///< Current smoothed gain value
  float deltaGain_;       ///< Per-sample increment
  bool needsSmoothing_;   ///< Flag to trigger smoothing
  int samplesPerBlock_;   ///< Samples per block for smoothing calculation
};

} // namespace ms
