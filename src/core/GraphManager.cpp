#include "MicroSuono/GraphManager.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>

namespace ms {

GraphManager::GraphManager() {}

GraphManager::~GraphManager() {}

NodePtr GraphManager::createNode(const std::string& id, NodePtr node) {
  std::lock_guard<std::mutex> lock(graphMutex_);
  
  if (nodes_.find(id) != nodes_.end()) {
    std::cerr << "Warning: Node with id '" << id << "' already exists!" << std::endl;
    return nodes_[id];
  }
  
  nodes_[id] = node;
  orderedNodes_.push_back(node);
  
  // If graph is already prepared, prepare this node and allocate its buffers
  if (isPrepared_) {
    node->setGraphManager(this);
    node->prepare(sampleRate_, blockSize_);
    allocateBuffersForNode(id);
    std::cout << "Node '" << id << "' hot-added to running graph." << std::endl;
  }
  
  return node;
}

bool GraphManager::removeNode(const std::string& id) {
  std::lock_guard<std::mutex> lock(graphMutex_);
  
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
  audioBuffers_.erase(id);
  controlValues_.erase(id);
  eventBuffers_.erase(id);
  
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

void GraphManager::connect(const std::string& fromId, const std::string& fromPort,
                           const std::string& toId, const std::string& toPort) {
  std::lock_guard<std::mutex> lock(graphMutex_);
  
  if (nodes_.find(fromId) == nodes_.end()) {
    std::cerr << "Error: Source node '" << fromId << "' not found!" << std::endl;
    return;
  }
  if (nodes_.find(toId) == nodes_.end()) {
    std::cerr << "Error: Destination node '" << toId << "' not found!" << std::endl;
    return;
  }
  
  connections_.push_back({fromId, fromPort, toId, toPort});
  std::cout << "Connected: " << fromId << "." << fromPort << " -> " 
            << toId << "." << toPort << std::endl;
}

bool GraphManager::disconnect(const std::string& fromId, const std::string& fromPort,
                              const std::string& toId, const std::string& toPort) {
  std::lock_guard<std::mutex> lock(graphMutex_);
  
  auto it = std::remove_if(connections_.begin(), connections_.end(),
    [&](const Connection& conn) {
      return conn.fromNodeId == fromId && conn.fromPortName == fromPort &&
             conn.toNodeId == toId && conn.toPortName == toPort;
    });
  
  if (it != connections_.end()) {
    connections_.erase(it, connections_.end());
    std::cout << "Disconnected: " << fromId << "." << fromPort << " -> "
              << toId << "." << toPort << std::endl;
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
  audioBuffers_.clear();
  controlValues_.clear();
  eventBuffers_.clear();
  std::cout << "Graph cleared." << std::endl;
}

void GraphManager::prepare(int sampleRate, int blockSize) {
  std::lock_guard<std::mutex> lock(graphMutex_);
  
  sampleRate_ = sampleRate;
  blockSize_ = blockSize;
  
  for (auto& node : orderedNodes_) {
    node->setGraphManager(this);
    node->prepare(sampleRate, blockSize);
  }
  
  allocateBuffers();
  isPrepared_ = true;
  
  std::cout << "GraphManager prepared: " << sampleRate << "Hz, "
            << blockSize << " samples/block" << std::endl;
}

void GraphManager::allocateBuffers() {
  audioBuffers_.clear();
  controlValues_.clear();
  eventBuffers_.clear();
  
  for (auto& [id, node] : nodes_) {
    allocateBuffersForNode(id);
  }
}

void GraphManager::allocateBuffersForNode(const std::string& nodeId) {
  auto it = nodes_.find(nodeId);
  if (it == nodes_.end()) return;
  
  auto& node = it->second;
  std::vector<std::vector<float>> buffers;
  
  for (const auto& port : node->getOutputPorts()) {
    if (port.type == PortType::Audio) {
      buffers.push_back(std::vector<float>(blockSize_, 0.0f));
    }
  }
  
  if (!buffers.empty()) {
    audioBuffers_[nodeId] = std::move(buffers);
  }
  
  controlValues_[nodeId] = std::unordered_map<std::string, ControlValue>();
  eventBuffers_[nodeId] = std::unordered_map<std::string, std::vector<Event>>();
}

void GraphManager::process(int nFrames) {
  // Try to acquire lock - if graph is being modified, use previous state
  // This prevents audio glitches from blocking on mutex
  std::unique_lock<std::mutex> lock(graphMutex_, std::try_to_lock);
  if (!lock.owns_lock()) {
    // Graph is being modified - output silence this block to avoid race conditions
    // This is preferable to blocking the audio thread
    return;
  }
  
  // Clear event buffers at start of each block
  for (auto& [nodeId, eventPorts] : eventBuffers_) {
    for (auto& [portName, events] : eventPorts) {
      events.clear();
    }
  }
  
  for (auto& node : orderedNodes_) {
    const std::string& nodeId = node->id;
    
    std::unordered_map<std::string, ControlValue> controlInputs;
    std::unordered_map<std::string, std::vector<Event>> eventInputs;
    std::vector<const float*> audioInputPtrs;
    std::vector<float*> audioOutputPtrs;
    
    // Count connections per audio input port for summation
    std::vector<int> audioInputConnectionCount;
    std::vector<std::vector<const float*>> audioInputSources;
    
    // Initialize audio input structures
    int numAudioInputs = 0;
    for (const auto& port : node->getInputPorts()) {
      if (port.type == PortType::Audio) {
        numAudioInputs++;
      }
    }
    audioInputConnectionCount.resize(numAudioInputs, 0);
    audioInputSources.resize(numAudioInputs);
    
    // First pass: collect all audio connections to each input
    for (const auto& conn : connections_) {
      if (conn.toNodeId == nodeId) {
        auto fromNode = nodes_[conn.fromNodeId];
        
        int fromIdx = 0;
        for (const auto& fromPort : fromNode->getOutputPorts()) {
          if (fromPort.name == conn.fromPortName) {
            // Find target port type and index
            PortType toPortType = PortType::Audio;
            int toIdx = -1;
            int audioPortIdx = 0;
            for (const auto& toPort : node->getInputPorts()) {
              if (toPort.name == conn.toPortName) {
                toPortType = toPort.type;
                if (toPort.type == PortType::Audio) {
                  toIdx = audioPortIdx;
                }
                break;
              }
              if (toPort.type == PortType::Audio) audioPortIdx++;
            }
            
            if (fromPort.type == PortType::Audio && toPortType == PortType::Audio) {
              // Collect audio source for summation
              if (toIdx >= 0 && audioBuffers_.find(conn.fromNodeId) != audioBuffers_.end() && 
                  fromIdx < audioBuffers_[conn.fromNodeId].size()) {
                audioInputSources[toIdx].push_back(audioBuffers_[conn.fromNodeId][fromIdx].data());
                audioInputConnectionCount[toIdx]++;
              }
            }
            break;
          }
          if (fromPort.type == PortType::Audio) fromIdx++;
        }
      }
    }
    
    // Allocate summation buffers if needed
    int maxSummationBuffersNeeded = 0;
    for (int count : audioInputConnectionCount) {
      if (count > 1) maxSummationBuffersNeeded++;
    }
    
    if (maxSummationBuffersNeeded > (int)summationBuffers_.size()) {
      summationBuffers_.resize(maxSummationBuffersNeeded);
      for (auto& buf : summationBuffers_) {
        buf.resize(blockSize_, 0.0f);
      }
    }
    
    // Second pass: create audio input pointers with summation
    int summationBufferIdx = 0;
    for (int i = 0; i < numAudioInputs; ++i) {
      if (audioInputConnectionCount[i] == 0) {
        // No connections: null pointer
        audioInputPtrs.push_back(nullptr);
      } else if (audioInputConnectionCount[i] == 1) {
        // Single connection: direct pointer
        audioInputPtrs.push_back(audioInputSources[i][0]);
      } else {
        // Multiple connections: sum into summation buffer
        float* sumBuffer = summationBuffers_[summationBufferIdx].data();
        std::memset(sumBuffer, 0, nFrames * sizeof(float));
        
        for (const float* source : audioInputSources[i]) {
          for (int j = 0; j < nFrames; ++j) {
            sumBuffer[j] += source[j];
          }
        }
        
        audioInputPtrs.push_back(sumBuffer);
        summationBufferIdx++;
      }
    }
    
    // Process audio→control connections (as additional audio inputs)
    for (const auto& conn : connections_) {
      if (conn.toNodeId == nodeId) {
        auto fromNode = nodes_[conn.fromNodeId];
        
        int fromIdx = 0;
        for (const auto& fromPort : fromNode->getOutputPorts()) {
          if (fromPort.name == conn.fromPortName) {
            // Find target port type
            PortType toPortType = PortType::Audio;
            for (const auto& toPort : node->getInputPorts()) {
              if (toPort.name == conn.toPortName) {
                toPortType = toPort.type;
                break;
              }
            }
            
            if (fromPort.type == PortType::Audio && toPortType == PortType::Control) {
              // Audio output to Control input: add as additional audio input
              if (audioBuffers_.find(conn.fromNodeId) != audioBuffers_.end() && 
                  fromIdx < audioBuffers_[conn.fromNodeId].size()) {
                audioInputPtrs.push_back(audioBuffers_[conn.fromNodeId][fromIdx].data());
              }
            } else if (fromPort.type == PortType::Control) {
              if (controlValues_.find(conn.fromNodeId) != controlValues_.end()) {
                auto& fromControls = controlValues_[conn.fromNodeId];
                if (fromControls.find(conn.fromPortName) != fromControls.end()) {
                  controlInputs[conn.toPortName] = fromControls[conn.fromPortName];
                }
              }
            } else if (fromPort.type == PortType::Event) {
              if (eventBuffers_.find(conn.fromNodeId) != eventBuffers_.end()) {
                auto& fromEvents = eventBuffers_[conn.fromNodeId];
                if (fromEvents.find(conn.fromPortName) != fromEvents.end()) {
                  eventInputs[conn.toPortName] = fromEvents[conn.fromPortName];
                }
              }
            }
            break;
          }
          if (fromPort.type == PortType::Audio) fromIdx++;
        }
      }
    }
    
    int audioOutIdx = 0;
    for (const auto& port : node->getOutputPorts()) {
      if (port.type == PortType::Audio) {
        if (audioBuffers_.find(nodeId) != audioBuffers_.end() && 
            audioOutIdx < audioBuffers_[nodeId].size()) {
          audioOutputPtrs.push_back(audioBuffers_[nodeId][audioOutIdx].data());
          audioOutIdx++;
        }
      }
    }
    
    std::unordered_map<std::string, ControlValue> controlOutputs;
    node->processControl(controlInputs, controlOutputs);
    controlValues_[nodeId] = controlOutputs;
    
    std::unordered_map<std::string, std::vector<Event>> eventOutputs;
    node->processEvents(eventInputs, eventOutputs);
    eventBuffers_[nodeId] = eventOutputs;
    
    node->process(
      audioInputPtrs.empty() ? nullptr : audioInputPtrs.data(), 
      audioOutputPtrs.empty() ? nullptr : audioOutputPtrs.data(), 
      nFrames
    );
  }
}

const float* GraphManager::getNodeOutput(const std::string& nodeId, int outputIndex) const {
  auto it = audioBuffers_.find(nodeId);
  if (it != audioBuffers_.end() && outputIndex < static_cast<int>(it->second.size())) {
    return it->second[outputIndex].data();
  }
  return nullptr;
}

void GraphManager::setPhysicalInput(int channelIndex, const float* data, int nFrames) {
  // Resize if needed
  if (channelIndex >= (int)physicalInputBuffers_.size()) {
    physicalInputBuffers_.resize(channelIndex + 1);
  }
  
  // Resize buffer if needed
  if (physicalInputBuffers_[channelIndex].size() != (size_t)nFrames) {
    physicalInputBuffers_[channelIndex].resize(nFrames);
  }
  
  // Copy data
  std::memcpy(physicalInputBuffers_[channelIndex].data(), data, nFrames * sizeof(float));
}

const float* GraphManager::getPhysicalInput(int channelIndex) const {
  if (channelIndex >= 0 && channelIndex < (int)physicalInputBuffers_.size()) {
    return physicalInputBuffers_[channelIndex].data();
  }
  return nullptr;
}

} // namespace ms
