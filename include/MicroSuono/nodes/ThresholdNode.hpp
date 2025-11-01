#pragma once
#include "../Node.hpp"

namespace ms {

/**
 * Threshold detector node
 * Generates trigger events when audio signal crosses threshold
 */
class ThresholdNode : public Node {
public:
  ThresholdNode(const std::string& id, float threshold = 0.5f);
  
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  void processEvents(
    const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
    std::unordered_map<std::string, std::vector<Event>>& eventOutputs) override;
  
  void setThreshold(float threshold);
  float getThreshold() const;

protected:
  float threshold_;              ///< Threshold level
  bool wasAboveThreshold_;       ///< Track previous state for edge detection
};

} // namespace ms
