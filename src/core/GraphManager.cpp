#include "MicroSuono/GraphManager.hpp"
#include <iostream>
#include <string>

namespace ms {
//
// GraphManager::GraphManager() {}
//
// GraphManager::~GraphManager() {}
//
// NodePtr GraphManager::createNode(const std::string &id, NodePtr node) {
//   if (nodes_.find(id) != nodes_.end()) {
//     std::cerr << "Node with id " << id << " already exists." << std::endl;
//     return nodes_[id];
//   }
//   nodes_[id] = node;
//   return node;
// }
//
// void GraphManager::connect(const std::string &fromId, int fromOutput,
//                            const std::string &toId, int toInput) {
//   if (nodes_.find(fromId) == nodes_.end()) {
//     std::cerr << "Source node " << fromId << " does not exist." << std::endl;
//     return;
//   }
//   if (nodes_.find(toId) == nodes_.end()) {
//     std::cerr << "Destination node " << toId << " does not exist." <<
//     std::endl; return;
//   }
//   connections_.push_back({fromId, fromOutput, toId, toInput});
//   std::cout << "Connected " << fromId << " output " << fromOutput << " to "
//             << toId << " input " << toInput << std::endl;
// }
//
// void GraphManager::prepare(int sampleRate, int blockSize) {
//   sampleRate_ = sampleRate;
//   blockSize_ = blockSize;
//
//   for (auto &[id, node] : nodes_) {
//     node->prepare(sampleRate, blockSize);
//   }
//   allocateBuffers();
//   std::cout << "Graph prepared with sample rate " << sampleRate
//             << " and block size " << blockSize << std::endl;
// }
//
} // namespace ms
