#pragma once
#include <string>
#include <vector>

namespace ms {

/** Node parameter structure */
struct Param {
  std::string name;
  float value;
};

/** Abstract base class for all audio/signal processing nodes */
class Node {
public:
  Node(const std::string &id, int nInputs = 1, int nOutputs = 1)
    : id(id), nInputs(nInputs), nOutputs(nOutputs) {}

  virtual ~Node() = default;

  /** Non-realtime preparation (buffer allocation, initialization) */
  virtual void prepare(int sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
  }

  /** 
   * Realtime audio processing
   * @param inputs Array of input buffer pointers
   * @param outputs Array of output buffer pointers
   * @param nFrames Number of frames to process
   */
  virtual void process(const float *const *inputs, float **outputs, int nFrames) = 0;

  const std::string id;
  int nInputs;
  int nOutputs;
  std::vector<Param> params;

protected:
  int sampleRate_ = 44100;
  int blockSize_ = 512;
};

} // namespace ms
