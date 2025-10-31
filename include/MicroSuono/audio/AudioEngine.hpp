#pragma once
#include "MicroSuono/GraphManager.hpp"
#include "miniaudio.hpp"
#include <string>
#include <vector>

namespace ms {

/**
 * @brief Manages audio device and real-time audio streaming.
 *
 * AudioEngine handles initialization, streaming, and mapping of graph node outputs to physical audio channels. It supports fade-out, input injection, and device management using miniaudio.
 */
class AudioEngine {
public:
  /**
   * @brief Construct a new AudioEngine
   * @param graph Pointer to GraphManager instance
   */
  explicit AudioEngine(GraphManager* graph);
  /**
   * @brief Destroy the AudioEngine
   */
  ~AudioEngine();

  /**
   * @brief Initialize and start the audio device
   * @param sampleRate Sample rate in Hz
   * @param blockSize Number of frames per callback
   * @param numOutputChannels Number of output channels (default: 2 for stereo)
   * @param numInputChannels Number of input channels (default: 0, no input)
   * @return True if device started successfully, false otherwise
   */
  bool start(int sampleRate = 44100, int blockSize = 512, int numOutputChannels = 2, int numInputChannels = 0);
  
  /**
   * @brief Stop the audio device with optional fade-out
   * @param fadeOutMs Fade-out duration in milliseconds (0 = immediate stop, no fade)
   */
  void stop(float fadeOutMs = 0.0f);

  /**
   * @brief Set default fade-out duration for stop()
   * @param fadeOutMs Fade-out duration in milliseconds
   */
  void setFadeOutDuration(float fadeOutMs) { fadeOutDurationMs_ = fadeOutMs; }

  /**
   * @brief Map a graph node output to a physical audio output channel
   * @param channelIndex Physical output channel (0 = left, 1 = right, etc.)
   * @param nodeId Node ID to read from
   * @param outputIndex Output port index of the node (default: 0)
   */
  void mapOutputChannel(int channelIndex, const std::string& nodeId, int outputIndex = 0);

  /**
   * @brief Clear all output channel mappings
   */
  void clearOutputChannelMappings();

private:
  static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  struct ChannelMapping {
    std::string nodeId;
    int portIndex;
  };

  GraphManager* graph_;
  ma_device device_;
  int blockSize_;
  int numOutputChannels_;
  int numInputChannels_;
  std::vector<ChannelMapping> outputChannelMappings_;
  
  // Fade-out state
  float fadeOutDurationMs_ = 0.0f;
  int fadeOutSamples_ = 0;
  int currentFadeSample_ = 0;
  bool fadeOutActive_ = false;
};

} // namespace ms
