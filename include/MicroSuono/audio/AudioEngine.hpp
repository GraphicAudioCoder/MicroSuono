#pragma once
#include "MicroSuono/GraphManager.hpp"
#include "miniaudio.hpp"

namespace ms {

class AudioEngine {
public:
  AudioEngine(GraphManager *graph);
  ~AudioEngine();

  bool start(int sampleRate = 44100, int blockSize = 512);
  void stop();

private:
  static void audioCallback(ma_device *pDevice, void *pOutput,
                            const void *pInput, ma_uint32 frameCount);

  GraphManager *graph_;
  ma_device device_;
  int blockSize_;
};

} // namespace ms
