#include "MicroSuono/Node.hpp"
#include "MicroSuono/GraphManager.hpp"

namespace ms {

const float* Node::getPhysicalInput(int channelIndex) const {
  if (graphManager_) {
    return graphManager_->getPhysicalInput(channelIndex);
  }
  return nullptr;
}

} // namespace ms
