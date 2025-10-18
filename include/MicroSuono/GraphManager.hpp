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

/** Connection between two nodes */
struct Connection {
  std::string fromNodeId;
  std::string fromPortName;
  std::string toNodeId;
  std::string toPortName;
};

/** Manages the audio processing graph and node connections */
class GraphManager {
public:
  GraphManager();
  ~GraphManager();

  /** Add a node to the graph */
  NodePtr createNode(const std::string &id, NodePtr node);
  
  /** Remove a node and all its connections */
  bool removeNode(const std::string &id);
  
  /** Get a node by ID */
  NodePtr getNode(const std::string &id) const;
  
  /** Connect two nodes by port name */
  void connect(const std::string &fromId, const std::string &fromPort,
               const std::string &toId, const std::string &toPort);
  
  /** Disconnect a specific connection */
  bool disconnect(const std::string &fromId, const std::string &fromPort,
                  const std::string &toId, const std::string &toPort);
  
  /** Remove all connections for a node */
  void disconnectAll(const std::string &nodeId);
  
  /** Clear the entire graph */
  void clear();

  /** Prepare all nodes for processing */
  void prepare(int sampleRate, int blockSize);
  
  /** Process all nodes in the graph */
  void process(int nFrames);

  /** Get the output buffer of a specific node */
  const float *getNodeOutput(const std::string &nodeId, int outputIndex = 0) const;

  /** Set physical audio input data (called by AudioEngine before processing)
   * @param channelIndex Physical input channel index
   * @param data Input audio data
   * @param nFrames Number of frames
   */
  void setPhysicalInput(int channelIndex, const float* data, int nFrames);

  /** Get physical audio input buffer (for nodes to read from)
   * @param channelIndex Physical input channel index
   * @return Pointer to input buffer, or nullptr if invalid
   */
  const float* getPhysicalInput(int channelIndex) const;

  /** Get number of physical input channels */
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
