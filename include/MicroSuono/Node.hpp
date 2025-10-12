#pragma once
#include <string>
#include <vector>

namespace ms {

// Generic parameter for a node
struct Param {
  std::string name;
  float value;
};

// Abstract base node class
class Node {
public:
  Node(const std::string &id, int nInputs = 1, int nOutputs = 1)
      : id(id), nInputs(nInputs), nOutputs(nOutputs) {}

  virtual ~Node() = default;

  // Non-realtime preparation (init, buffer allocation, heavy calculations)
  virtual void prepare(int sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
  }

  // Realtime audio/value processing
  // inputs[i] = pointer to the i-th input buffer
  // outputs[i] = pointer to the i-th output buffer
  virtual void process(const float *const *inputs, float **outputs,
                       int nFrames) = 0;

  const std::string id;
  int nInputs;
  int nOutputs;
  std::vector<Param> params;

protected:
  int sampleRate_ = 44100;
  int blockSize_ = 512;
};

} // namespace ms
