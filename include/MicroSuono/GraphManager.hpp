#pragma once
#include "Node.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace ms {

using NodePtr = std::shared_ptr<Node>;

struct Connection {
  std::string fromNodeId;
  int fromOutput;
  std::string toNodeId;
  int toInput;
};

class GraphManager {
public:
  GraphManager();
  ~GraphManager();

  NodePtr createNode(const std::string &id, NodePtr node);
  void connect(const std::string &fromId, int fromOutput,
               const std::string &toId, int toInput);

  void prepare(int sampleRate, int blockSize);
  void process(int nFrames);


    // Get the final output of a specific node
    const float *getNodeOutput(const std::string &nodeId,
                               int outputIndex = 0) const;

private:
  void allocateBuffers();
  void sortNodes();

  std::unordered_map<std::string, NodePtr> nodes_;
  std::vector<NodePtr> orderedNodes_;
  std::vector<Connection> connections_;


  // Internal buffers for each node
  std::unordered_map<std::string, std::vector<std::vector<float>>> nodeBuffers_;

  int sampleRate_ = 44100;
  int blockSize_ = 512;
};

} // namespace ms
