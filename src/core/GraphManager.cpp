#include "MicroSuono/GraphManager.hpp"
#include <algorithm>
#include <iostream>

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

void GraphManager::prepare(int sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
    
    // Prepare each node
    for (auto& node : orderedNodes_) {
        node->prepare(sampleRate, blockSize);
    }
    // Allocate buffers for each node
    allocateBuffers();
    std::cout << "GraphManager prepared: " << sampleRate << "Hz, "
                        << blockSize << " samples/block" << std::endl;
}

void GraphManager::allocateBuffers() {
    nodeBuffers_.clear();
    
    for (auto& [id, node] : nodes_) {
        std::vector<std::vector<float>> buffers;
        
        // Alloca un buffer per ogni output del nodo
        for (int i = 0; i < node->nOutputs; ++i) {
            buffers.push_back(std::vector<float>(blockSize_, 0.0f));
        }
        
        nodeBuffers_[id] = std::move(buffers);
    }
}

void GraphManager::process(int nFrames) {
    // For each node in order
    for (auto& node : orderedNodes_) {
        const std::string& nodeId = node->id;
        // Prepare node inputs
        std::vector<const float*> inputPtrs(node->nInputs, nullptr);
        // Find input connections for this node
        for (const auto& conn : connections_) {
            if (conn.toNodeId == nodeId) {
                // Point to the output buffer of the source node
                if (nodeBuffers_.find(conn.fromNodeId) != nodeBuffers_.end()) {
                    inputPtrs[conn.toInput] = nodeBuffers_[conn.fromNodeId][conn.fromOutput].data();
                }
            }
        }
        // Prepare node outputs
        std::vector<float*> outputPtrs(node->nOutputs);
        for (int i = 0; i < node->nOutputs; ++i) {
            outputPtrs[i] = nodeBuffers_[nodeId][i].data();
        }
        // Process the node
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
