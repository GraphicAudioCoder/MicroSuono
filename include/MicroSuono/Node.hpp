#pragma once
#include "Port.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace ms {

/** Node parameter structure */
struct Param {
  std::string name;
  float value;
};

/** Abstract base class for all audio/signal processing nodes */
class Node {
public:
  Node(const std::string &id)
    : id(id) {}

  virtual ~Node() = default;

  /** Non-realtime preparation (buffer allocation, initialization) */
  virtual void prepare(int sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
  }

  /** 
   * Realtime audio processing
   * @param audioInputs Array of audio input buffer pointers
   * @param audioOutputs Array of audio output buffer pointers
   * @param nFrames Number of frames to process
   */
  virtual void process(const float *const *audioInputs, float **audioOutputs, int nFrames) = 0;

  /** 
   * Process control messages (called once per block)
   * @param controlInputs Map of control input values by port name
   * @param controlOutputs Map to write control output values
   */
  virtual void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) {}

  /** Get input port descriptors */
  const std::vector<Port>& getInputPorts() const { return inputPorts_; }
  
  /** Get output port descriptors */
  const std::vector<Port>& getOutputPorts() const { return outputPorts_; }

  const std::string id;
  std::vector<Param> params;

protected:
  void addInputPort(const std::string& name, PortType type) {
    inputPorts_.push_back(Port(name, type));
  }
  
  void addOutputPort(const std::string& name, PortType type) {
    outputPorts_.push_back(Port(name, type));
  }

  std::vector<Port> inputPorts_;
  std::vector<Port> outputPorts_;
  
  int sampleRate_ = 44100;
  int blockSize_ = 512;
};

} // namespace ms
