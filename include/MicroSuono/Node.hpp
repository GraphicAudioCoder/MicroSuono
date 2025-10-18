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
    // Calculate fade-in samples from duration
    updateFadeInSamples();
    currentFadeSample_ = 0;
    fadeInActive_ = (fadeInDurationMs_ > 0.0f);
  }

  /** Set fade-in duration in milliseconds
   * @param durationMs Fade-in duration (0 = disabled, default = 50ms)
   */
  void setFadeInDuration(float durationMs) {
    fadeInDurationMs_ = durationMs;
    updateFadeInSamples();
  }

  /** Get current fade-in duration in milliseconds */
  float getFadeInDuration() const { return fadeInDurationMs_; }

  /** Reset fade-in (useful when re-activating a node) */
  void resetFadeIn() {
    currentFadeSample_ = 0;
    fadeInActive_ = (fadeInDurationMs_ > 0.0f);
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

  /**
   * Process events (called once per block, before audio processing)
   * @param eventInputs Map of event queues by port name
   * @param eventOutputs Map to write output events by port name
   */
  virtual void processEvents(
    const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
    std::unordered_map<std::string, std::vector<Event>>& eventOutputs) {}

  /** Get input port descriptors */
  const std::vector<Port>& getInputPorts() const { return inputPorts_; }
  
  /** Get output port descriptors */
  const std::vector<Port>& getOutputPorts() const { return outputPorts_; }

  const std::string id;
  std::vector<Param> params;

  /** Set GraphManager reference (called by GraphManager during setup)
   * This allows nodes to access physical inputs
   */
  void setGraphManager(class GraphManager* graph) { graphManager_ = graph; }

protected:
  /** Apply fade-in envelope to an audio buffer (called internally)
   * @param buffer Audio buffer to apply fade-in to
   * @param nFrames Number of frames in the buffer
   */
  void applyFadeIn(float* buffer, int nFrames) {
    if (!fadeInActive_) return;
    
    for (int i = 0; i < nFrames; ++i) {
      if (currentFadeSample_ < fadeInSamples_) {
        float fadeGain = static_cast<float>(currentFadeSample_) / fadeInSamples_;
        buffer[i] *= fadeGain;
        currentFadeSample_++;
      } else {
        fadeInActive_ = false;
        break;
      }
    }
  }
  void addInputPort(const std::string& name, PortType type) {
    inputPorts_.push_back(Port(name, type));
  }
  
  void addOutputPort(const std::string& name, PortType type) {
    outputPorts_.push_back(Port(name, type));
  }

  /** Get physical audio input from hardware (for nodes that need direct hardware access)
   * @param channelIndex Physical input channel index
   * @return Pointer to input buffer, or nullptr if not available
   */
  const float* getPhysicalInput(int channelIndex) const;

  std::vector<Port> inputPorts_;
  std::vector<Port> outputPorts_;
  
  int sampleRate_ = 44100;
  int blockSize_ = 512;
  
  // Fade-in state
  float fadeInDurationMs_ = 50.0f;  // Default 50ms fade-in
  int fadeInSamples_ = 0;
  int currentFadeSample_ = 0;
  bool fadeInActive_ = false;
  
  class GraphManager* graphManager_ = nullptr;

private:
  /** Update fade-in samples from duration and sample rate */
  void updateFadeInSamples() {
    fadeInSamples_ = static_cast<int>((fadeInDurationMs_ / 1000.0f) * sampleRate_);
  }
};

} // namespace ms
