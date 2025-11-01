#pragma once
#include "MicroSuono/Node.hpp"

namespace ms {

/** Converts mono input to stereo output with panning control
 * 
 * Input ports:
 *  - "in" (Audio): Mono audio input
 *  - "pan" (Control): Pan position (-1.0 = left, 0.0 = center, +1.0 = right)
 * 
 * Output ports:
 *  - "left" (Audio): Left channel output
 *  - "right" (Audio): Right channel output
 */
class MonoToStereoNode : public Node {
public:
  /** Create mono to stereo converter
   * @param id Node identifier
   * @param pan Initial pan position (-1.0 to +1.0, default: 0.0 center)
   */
  explicit MonoToStereoNode(const std::string& id, float pan = 0.0f);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;

protected:
  float pan_;  ///< Pan position (-1.0 = left, 0.0 = center, +1.0 = right)
};

} // namespace ms
