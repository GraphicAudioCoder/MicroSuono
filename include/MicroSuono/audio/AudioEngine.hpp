#pragma once
#include "MicroSuono/GraphManager.hpp"
#include "miniaudio.hpp"
#include <string>
#include <vector>

namespace ms {

/** Manages audio device and real-time audio streaming */
class AudioEngine {
public:
  explicit AudioEngine(GraphManager* graph);
  ~AudioEngine();

  /** Initialize and start the audio device
   * @param sampleRate Sample rate in Hz
   * @param blockSize Number of frames per callback
   * @param numChannels Number of output channels (default: 2 for stereo)
   */
  bool start(int sampleRate = 44100, int blockSize = 512, int numChannels = 2);
  
  /** Stop the audio device */
  void stop();

  /** Map a graph node output to a physical audio channel
   * @param channelIndex Physical output channel (0 = left, 1 = right, etc.)
   * @param nodeId Node ID to read from
   * @param outputIndex Output port index of the node (default: 0)
   */
  void mapOutputChannel(int channelIndex, const std::string& nodeId, int outputIndex = 0);

  /** Clear all channel mappings */
  void clearChannelMappings();

private:
  static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  struct ChannelMapping {
    std::string nodeId;
    int outputIndex;
  };

  GraphManager* graph_;
  ma_device device_;
  int blockSize_;
  int numChannels_;
  std::vector<ChannelMapping> channelMappings_;
};

} // namespace ms
