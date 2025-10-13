#include "MicroSuono/audio/AudioEngine.hpp"
#include <cstring>
#include <iostream>

namespace ms {

AudioEngine::AudioEngine(GraphManager* graph)
  : graph_(graph), blockSize_(512) {
  std::memset(&device_, 0, sizeof(device_));
}

AudioEngine::~AudioEngine() {
  stop();
}

void AudioEngine::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  AudioEngine* engine = (AudioEngine*)pDevice->pUserData;
  if (!engine || !engine->graph_) return;

  float* output = (float*)pOutput;
  engine->graph_->process(frameCount);
  
  const float* graphOutput = engine->graph_->getNodeOutput("output", 0);
  if (graphOutput) {
    for (ma_uint32 i = 0; i < frameCount; ++i) {
      output[i] = graphOutput[i];
    }
  }
}

bool AudioEngine::start(int sampleRate, int blockSize) {
  blockSize_ = blockSize;
  
  ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = ma_format_f32;
  deviceConfig.playback.channels = 1;
  deviceConfig.sampleRate = sampleRate;
  deviceConfig.dataCallback = AudioEngine::audioCallback;
  deviceConfig.pUserData = this;

  if (ma_device_init(NULL, &deviceConfig, &device_) != MA_SUCCESS) {
    std::cerr << "Failed to initialize audio device." << std::endl;
    return false;
  }

  graph_->prepare(device_.sampleRate, blockSize_);

  if (ma_device_start(&device_) != MA_SUCCESS) {
    std::cerr << "Failed to start audio device." << std::endl;
    ma_device_uninit(&device_);
    return false;
  }

  return true;
}

void AudioEngine::stop() {
  ma_device_uninit(&device_);
}

} // namespace ms
