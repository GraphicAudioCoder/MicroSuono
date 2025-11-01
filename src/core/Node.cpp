#include "MicroSuono/Node.hpp"
#include "MicroSuono/GraphManager.hpp"

namespace ms {

/**
 * @brief Apply fade-in envelope to an audio buffer.
 * 
 * This method implements a linear fade-in from 0 to 1 over the configured duration.
 * Subclasses should call this at the end of their process() method on the output buffer.
 * 
 * @param buffer Audio buffer to apply fade-in to
 * @param nFrames Number of frames in the buffer
 */
void Node::applyFadeIn(float* buffer, int nFrames) {
  if (!fadeInActive_) return;
  
  for (int i = 0; i < nFrames; ++i) {
    if (currentFadeSample_ < fadeInSamples_) {
      float fadeGain = static_cast<float>(currentFadeSample_) / fadeInSamples_;
      buffer[i] *= fadeGain;
      currentFadeSample_++;
    } else {
      fadeInActive_ = false;
      break;
    }
  }
}

/**
 * @brief Get pointer to physical audio input buffer from GraphManager.
 *
 * Used by nodes that require direct access to hardware input channels.
 * @param channelIndex Physical input channel index
 * @return Pointer to input buffer, or nullptr if unavailable
 */
const float* Node::getPhysicalInput(int channelIndex) const {
  if (graphManager_) {
    return graphManager_->getPhysicalInput(channelIndex);
  }
  return nullptr;
}

} // namespace ms
