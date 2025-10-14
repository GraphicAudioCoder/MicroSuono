#include "MicroSuono/audio/AudioEngine.hpp"
#include <cstring>
#include <iostream>

namespace ms {

AudioEngine::AudioEngine(GraphManager* graph)
  : graph_(graph), blockSize_(512), numChannels_(2) {
  std::memset(&device_, 0, sizeof(device_));
}

AudioEngine::~AudioEngine() {
  stop();
}

void AudioEngine::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  AudioEngine* engine = (AudioEngine*)pDevice->pUserData;
  if (!engine || !engine->graph_) return;

  float* output = (float*)pOutput;
  
  // Clear output buffer
  std::memset(output, 0, frameCount * engine->numChannels_ * sizeof(float));
  
  // Process graph
  engine->graph_->process(frameCount);
  
  // Map graph outputs to physical channels
  for (int ch = 0; ch < engine->numChannels_ && ch < (int)engine->channelMappings_.size(); ++ch) {
    const auto& mapping = engine->channelMappings_[ch];
    const float* graphOutput = engine->graph_->getNodeOutput(mapping.nodeId, mapping.outputIndex);
    
    if (graphOutput) {
      // Interleave channel data
      for (ma_uint32 i = 0; i < frameCount; ++i) {
        output[i * engine->numChannels_ + ch] = graphOutput[i];
      }
    }
  }
}

bool AudioEngine::start(int sampleRate, int blockSize, int numChannels) {
  blockSize_ = blockSize;
  numChannels_ = numChannels;
  
  ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = ma_format_f32;
  deviceConfig.playback.channels = numChannels_;
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

void AudioEngine::mapOutputChannel(int channelIndex, const std::string& nodeId, int outputIndex) {
  // Resize if needed
  if (channelIndex >= (int)channelMappings_.size()) {
    channelMappings_.resize(channelIndex + 1);
  }
  
  channelMappings_[channelIndex] = {nodeId, outputIndex};
}

void AudioEngine::clearChannelMappings() {
  channelMappings_.clear();
}

} // namespace ms
