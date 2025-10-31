#pragma once
#include "Node.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

namespace ms {

using NodePtr = std::shared_ptr<Node>;

/**
 * @brief Connection between two nodes in the graph.
 * Represents a directed link from an output port of one node to an input port of another node.
 */
struct Connection {
  std::string fromNodeId;    ///< Source node ID
  std::string fromPortName;  ///< Source port name
  std::string toNodeId;      ///< Destination node ID
  std::string toPortName;    ///< Destination port name
};

/**
 * @brief Manages the audio processing graph and node connections.
 *
 * GraphManager handles node creation, connection management, buffer allocation, and audio/control/event processing for the entire graph. Provides thread safety for dynamic modifications.
 */
class GraphManager {
public:
  /**
   * @brief Construct a new GraphManager
   */
  GraphManager();
  /**
   * @brief Destroy the GraphManager
   */
  ~GraphManager();

  /**
   * @brief Add a node to the graph
   * @param id Unique node identifier
   * @param node Shared pointer to node instance
   * @return NodePtr to the added node
   */
  NodePtr createNode(const std::string &id, NodePtr node);
  
  /**
   * @brief Remove a node and all its connections
   * @param id Node identifier
   * @return True if node was removed, false if not found
   */
  bool removeNode(const std::string &id);
  
  /**
   * @brief Get a node by ID
   * @param id Node identifier
   * @return Shared pointer to node, or nullptr if not found
   */
  NodePtr getNode(const std::string &id) const;
  
  /**
   * @brief Connect two nodes by port name
   * @param fromId Source node ID
   * @param fromPort Source port name
   * @param toId Destination node ID
   * @param toPort Destination port name
   */
  void connect(const std::string &fromId, const std::string &fromPort,
               const std::string &toId, const std::string &toPort);
  
  /**
   * @brief Disconnect a specific connection
   * @param fromId Source node ID
   * @param fromPort Source port name
   * @param toId Destination node ID
   * @param toPort Destination port name
   * @return True if connection was removed, false if not found
   */
  bool disconnect(const std::string &fromId, const std::string &fromPort,
                  const std::string &toId, const std::string &toPort);
  
  /**
   * @brief Remove all connections for a node
   * @param nodeId Node identifier
   */
  void disconnectAll(const std::string &nodeId);
  
  /**
   * @brief Clear the entire graph (removes all nodes and connections)
   */
  void clear();

  /**
   * @brief Prepare all nodes for processing
   * @param sampleRate Audio sample rate
   * @param blockSize Audio block size
   */
  void prepare(int sampleRate, int blockSize);
  
  /**
   * @brief Process all nodes in the graph (audio/control/event)
   * @param nFrames Number of frames to process
   */
  void process(int nFrames);

  /**
   * @brief Get the output buffer of a specific node
   * @param nodeId Node identifier
   * @param outputIndex Output port index (default 0)
   * @return Pointer to output buffer, or nullptr if not found
   */
  const float *getNodeOutput(const std::string &nodeId, int outputIndex = 0) const;

  /**
   * @brief Set physical audio input data (called by AudioEngine before processing)
   * @param channelIndex Physical input channel index
   * @param data Input audio data
   * @param nFrames Number of frames
   */
  void setPhysicalInput(int channelIndex, const float* data, int nFrames);

  /**
   * @brief Get physical audio input buffer (for nodes to read from)
   * @param channelIndex Physical input channel index
   * @return Pointer to input buffer, or nullptr if invalid
   */
  const float* getPhysicalInput(int channelIndex) const;

  /**
   * @brief Get number of physical input channels
   * @return Number of input channels
   */
  int getNumPhysicalInputs() const { return physicalInputBuffers_.size(); }

private:
  void allocateBuffers();
  void sortNodes();
  void allocateBuffersForNode(const std::string& nodeId);

  std::unordered_map<std::string, NodePtr> nodes_;
  std::vector<NodePtr> orderedNodes_;
  std::vector<Connection> connections_;
  std::unordered_map<std::string, std::vector<std::vector<float>>> audioBuffers_;
  std::unordered_map<std::string, std::unordered_map<std::string, ControlValue>> controlValues_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> eventBuffers_;
  
  // Physical audio input buffers (from hardware)
  std::vector<std::vector<float>> physicalInputBuffers_;
  
  // Summation buffers for multiple connections to same input
  std::vector<std::vector<float>> summationBuffers_;

  int sampleRate_ = 44100;
  int blockSize_ = 512;
  bool isPrepared_ = false;
  
  // Thread safety for dynamic modifications
  mutable std::mutex graphMutex_;
  std::atomic<bool> needsBufferReallocation_{false};
};

} // namespace ms
