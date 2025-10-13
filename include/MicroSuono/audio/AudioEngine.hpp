#pragma once
#include "MicroSuono/GraphManager.hpp"
#include "miniaudio.hpp"

namespace ms {

/** Manages audio device and real-time audio streaming */
class AudioEngine {
public:
  explicit AudioEngine(GraphManager* graph);
  ~AudioEngine();

  /** Initialize and start the audio device */
  bool start(int sampleRate = 44100, int blockSize = 512);
  
  /** Stop the audio device */
  void stop();

private:
  static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  GraphManager* graph_;
  ma_device device_;
  int blockSize_;
};

} // namespace ms
