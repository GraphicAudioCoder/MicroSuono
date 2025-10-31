#include "MicroSuono/Node.hpp"
#include "MicroSuono/GraphManager.hpp"

namespace ms {


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
