#pragma once
#include "Node.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

private:
  void allocateBuffers();
  void sortNodes();

  std::unordered_map<std::string, NodePtr> nodes_;
  std::vector<NodePtr> orderedNodes_;
  std::vector<Connection> connections_;
  std::unordered_map<std::string, std::vector<std::vector<float>>> audioBuffers_;
  std::unordered_map<std::string, std::unordered_map<std::string, ControlValue>> controlValues_;

  int sampleRate_ = 44100;
  int blockSize_ = 512;
};

} // namespace ms
