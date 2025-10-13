#include "MicroSuono/GraphManager.hpp"
#include <algorithm>
#include <iostream>
#include <string>

namespace ms {

GraphManager::GraphManager() {}

GraphManager::~GraphManager() {}

NodePtr GraphManager::createNode(const std::string& id, NodePtr node) {
  if (nodes_.find(id) != nodes_.end()) {
    std::cerr << "Warning: Node with id '" << id << "' already exists!" << std::endl;
    return nodes_[id];
  }
  
  nodes_[id] = node;
  orderedNodes_.push_back(node);
  return node;
}

bool GraphManager::removeNode(const std::string& id) {
  auto it = nodes_.find(id);
  if (it == nodes_.end()) {
    std::cerr << "Warning: Node '" << id << "' not found!" << std::endl;
    return false;
  }
  
  disconnectAll(id);
  
  orderedNodes_.erase(
    std::remove_if(orderedNodes_.begin(), orderedNodes_.end(),
      [&id](const NodePtr& node) { return node->id == id; }),
    orderedNodes_.end()
  );
  
  nodes_.erase(it);
  nodeBuffers_.erase(id);
  
  std::cout << "Node '" << id << "' removed." << std::endl;
  return true;
}

NodePtr GraphManager::getNode(const std::string& id) const {
  auto it = nodes_.find(id);
  if (it != nodes_.end()) {
    return it->second;
  }
  return nullptr;
}

void GraphManager::connect(const std::string& fromId, int fromOutput,
                           const std::string& toId, int toInput) {
  if (nodes_.find(fromId) == nodes_.end()) {
    std::cerr << "Error: Source node '" << fromId << "' not found!" << std::endl;
    return;
  }
  if (nodes_.find(toId) == nodes_.end()) {
    std::cerr << "Error: Destination node '" << toId << "' not found!" << std::endl;
    return;
  }
  
  connections_.push_back({fromId, fromOutput, toId, toInput});
  std::cout << "Connected: " << fromId << "[" << fromOutput << "] -> " 
            << toId << "[" << toInput << "]" << std::endl;
}

bool GraphManager::disconnect(const std::string& fromId, int fromOutput,
                              const std::string& toId, int toInput) {
  auto it = std::remove_if(connections_.begin(), connections_.end(),
    [&](const Connection& conn) {
      return conn.fromNodeId == fromId && conn.fromOutput == fromOutput &&
             conn.toNodeId == toId && conn.toInput == toInput;
    });
  
  if (it != connections_.end()) {
    connections_.erase(it, connections_.end());
    std::cout << "Disconnected: " << fromId << "[" << fromOutput << "] -> "
              << toId << "[" << toInput << "]" << std::endl;
    return true;
  }
  
  std::cerr << "Warning: Connection not found!" << std::endl;
  return false;
}

void GraphManager::disconnectAll(const std::string& nodeId) {
  connections_.erase(
    std::remove_if(connections_.begin(), connections_.end(),
      [&nodeId](const Connection& conn) {
        return conn.fromNodeId == nodeId || conn.toNodeId == nodeId;
      }),
    connections_.end()
  );
  std::cout << "All connections for node '" << nodeId << "' removed." << std::endl;
}

void GraphManager::clear() {
  nodes_.clear();
  orderedNodes_.clear();
  connections_.clear();
  nodeBuffers_.clear();
  std::cout << "Graph cleared." << std::endl;
}

void GraphManager::prepare(int sampleRate, int blockSize) {
  sampleRate_ = sampleRate;
  blockSize_ = blockSize;
  
  for (auto& node : orderedNodes_) {
    node->prepare(sampleRate, blockSize);
  }
  
  allocateBuffers();
  
  std::cout << "GraphManager prepared: " << sampleRate << "Hz, "
            << blockSize << " samples/block" << std::endl;
}

void GraphManager::allocateBuffers() {
  nodeBuffers_.clear();
  
  for (auto& [id, node] : nodes_) {
    std::vector<std::vector<float>> buffers;
    for (int i = 0; i < node->nOutputs; ++i) {
      buffers.push_back(std::vector<float>(blockSize_, 0.0f));
    }
    nodeBuffers_[id] = std::move(buffers);
  }
}

void GraphManager::process(int nFrames) {
  for (auto& node : orderedNodes_) {
    const std::string& nodeId = node->id;
    
    std::vector<const float*> inputPtrs(node->nInputs, nullptr);
    
    for (const auto& conn : connections_) {
      if (conn.toNodeId == nodeId) {
        if (nodeBuffers_.find(conn.fromNodeId) != nodeBuffers_.end()) {
          inputPtrs[conn.toInput] = nodeBuffers_[conn.fromNodeId][conn.fromOutput].data();
        }
      }
    }
    
    std::vector<float*> outputPtrs(node->nOutputs);
    for (int i = 0; i < node->nOutputs; ++i) {
      outputPtrs[i] = nodeBuffers_[nodeId][i].data();
    }
    
    node->process(inputPtrs.data(), outputPtrs.data(), nFrames);
  }
}

const float* GraphManager::getNodeOutput(const std::string& nodeId, int outputIndex) const {
  auto it = nodeBuffers_.find(nodeId);
  if (it != nodeBuffers_.end() && outputIndex < static_cast<int>(it->second.size())) {
    return it->second[outputIndex].data();
  }
  return nullptr;
}

} // namespace ms
