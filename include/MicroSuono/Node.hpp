#pragma once
#include "Port.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace ms {

/**
 * @brief Node parameter structure
 * Represents a named parameter for a node, holding a value of flexible type (float, int, bool, string).
 */
struct Param {
  std::string name;
  ControlValue value;
  
  /** Constructor for convenience */
  Param(const std::string& name, const ControlValue& value)
    : name(name), value(value) {}
};

/**
 * @brief Abstract base class for all audio/signal processing nodes.
 *
 * A Node represents a processing unit in the audio graph. It can have input/output ports of various types (audio, control, event), parameters, and supports fade-in envelopes for smooth activation. Derived classes implement the actual processing logic.
 */
class Node {
public:
  /**
   * @brief Construct a new Node
   * @param id Unique identifier for the node
   */
  Node(const std::string &id)
    : id_(id) {}

  /**
   * @brief Virtual destructor
   */
  virtual ~Node() = default;
  
  /**
   * @brief Get node unique identifier
   * @return Node ID string
   */
  const std::string& getId() const { return id_; }
  
  /**
   * @brief Get node parameters (read-only)
   * @return Vector of Param structures
   */
  const std::vector<Param>& getParams() const { return params_; }
  
  /**
   * @brief Get node parameters (mutable)
   * @return Vector of Param structures (modifiable)
   */
  std::vector<Param>& getParams() { return params_; }
  
  /**
   * @brief Set a parameter value by name
   * @param name Parameter name
   * @param value New value (ControlValue)
   * @return True if parameter found and set, false otherwise
   */
  bool setParam(const std::string& name, const ControlValue& value) {
    for (auto& param : params_) {
      if (param.name == name) {
        param.value = value;
        return true;
      }
    }
    return false;
  }
  
  /**
   * @brief Get a parameter value by name
   * @param name Parameter name
   * @return Pointer to value if found, nullptr otherwise
   */
  const ControlValue* getParam(const std::string& name) const {
    for (const auto& param : params_) {
      if (param.name == name) {
        return &param.value;
      }
    }
    return nullptr;
  }

  /**
   * @brief Non-realtime preparation (buffer allocation, initialization)
   * @param sampleRate Audio sample rate
   * @param blockSize Audio block size
   */
  virtual void prepare(int sampleRate, int blockSize) {
    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
    // Calculate fade-in samples from duration
    updateFadeInSamples();
    currentFadeSample_ = 0;
    fadeInActive_ = (fadeInDurationMs_ > 0.0f);
  }

  /**
   * @brief Set fade-in duration in milliseconds
   * @param durationMs Fade-in duration (0 = disabled, default = 50ms)
   */
  void setFadeInDuration(float durationMs) {
    fadeInDurationMs_ = durationMs;
    updateFadeInSamples();
  }

  /**
   * @brief Get current fade-in duration in milliseconds
   * @return Fade-in duration in ms
   */
  float getFadeInDuration() const { return fadeInDurationMs_; }

  /**
   * @brief Reset fade-in envelope (useful when re-activating a node)
   */
  void resetFadeIn() {
    currentFadeSample_ = 0;
    fadeInActive_ = (fadeInDurationMs_ > 0.0f);
  }

  /**
   * @brief Realtime audio processing (pure virtual)
   * @param audioInputs Array of audio input buffer pointers
   * @param audioOutputs Array of audio output buffer pointers
   * @param nFrames Number of frames to process
   */
  virtual void process(const float *const *audioInputs, float **audioOutputs, int nFrames) = 0;

  /**
   * @brief Process control messages (called once per block)
   * @param controlInputs Map of control input values by port name
   * @param controlOutputs Map to write control output values
   */
  virtual void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) {}

  /**
   * @brief Process events (called once per block, before audio processing)
   * @param eventInputs Map of event queues by port name
   * @param eventOutputs Map to write output events by port name
   */
  virtual void processEvents(
    const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
    std::unordered_map<std::string, std::vector<Event>>& eventOutputs) {}

  /**
   * @brief Get input port descriptors
   * @return Vector of input Port structures
   */
  const std::vector<Port>& getInputPorts() const { return inputPorts_; }
  
  /**
   * @brief Get output port descriptors
   * @return Vector of output Port structures
   */
  const std::vector<Port>& getOutputPorts() const { return outputPorts_; }

  /**
   * @brief Set GraphManager reference (called by GraphManager during setup)
   * This allows nodes to access physical inputs
   * @param graph Pointer to GraphManager
   */
  void setGraphManager(class GraphManager* graph) { graphManager_ = graph; }

protected:
  // === METHODS: Only what subclasses MUST call (cannot implement themselves) ===
  
  /**
   * @brief Apply fade-in envelope to an audio buffer
   * Subclasses should call this at the end of process() on their output buffer
   * @param buffer Audio buffer to apply fade-in to
   * @param nFrames Number of frames in the buffer
   */
  void applyFadeIn(float* buffer, int nFrames);
  
  /**
   * @brief Add an input port (must be called in constructor)
   * @param name Port name
   * @param type Port type (audio, control, event)
   */
  void addInputPort(const std::string& name, PortType type) {
    inputPorts_.push_back(Port(name, type));
  }
  
  /**
   * @brief Add an output port (must be called in constructor)
   * @param name Port name
   * @param type Port type (audio, control, event)
   */
  void addOutputPort(const std::string& name, PortType type) {
    outputPorts_.push_back(Port(name, type));
  }

  /**
   * @brief Get physical audio input from hardware
   * For nodes that need direct hardware access (e.g., AudioInputNode)
   * @param channelIndex Physical input channel index
   * @return Pointer to input buffer, or nullptr if not available
   */
  const float* getPhysicalInput(int channelIndex) const;

  // === DATA: Direct access for subclasses (for performance in process()) ===
  
  std::vector<Port> inputPorts_;   ///< Input port descriptors (read-only access recommended)
  std::vector<Port> outputPorts_;  ///< Output port descriptors (read-only access recommended)

  int sampleRate_ = 44100;         ///< Current audio sample rate
  int blockSize_ = 512;            ///< Current audio block size

  class GraphManager* graphManager_ = nullptr; ///< Reference to graph manager (for getPhysicalInput)

private:
  // === PRIVATE: Implementation details, not for subclasses ===
  
  std::string id_;              ///< Node unique identifier (access via getId())
  std::vector<Param> params_;   ///< Node parameters (access via getParams())
  
  // Fade-in state (managed internally, subclasses only call applyFadeIn())
  float fadeInDurationMs_ = 50.0f;  ///< Default 50ms fade-in
  int fadeInSamples_ = 0;           ///< Number of fade-in samples
  int currentFadeSample_ = 0;       ///< Current fade-in sample position
  bool fadeInActive_ = false;       ///< Is fade-in currently active?
  
  /**
   * @brief Update fade-in sample count from duration and sample rate
   */
  void updateFadeInSamples() {
    fadeInSamples_ = static_cast<int>((fadeInDurationMs_ / 1000.0f) * sampleRate_);
  }
};

} // namespace ms
