#pragma once
#include "MicroSuono/Node.hpp"

namespace ms {

/** Audio input node - receives audio from physical input (microphone, line-in, etc.)
 * 
 * This node reads directly from GraphManager's physical input buffers.
 * No special registration needed - it just works!
 * 
 * Output ports:
 *  - "out" (Audio): Audio signal from input device
 */
class AudioInputNode : public Node {
public:
  /** Create audio input node
   * @param id Node identifier
   * @param channelIndex Physical input channel to read from (default: 0)
   */
  explicit AudioInputNode(const std::string& id, int channelIndex = 0);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  
  int getChannelIndex() const { return channelIndex_; }
  void setChannelIndex(int index) { channelIndex_ = index; }

private:
  int channelIndex_;
};

} // namespace ms
