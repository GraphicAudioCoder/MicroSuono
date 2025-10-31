#include "MicroSuono/audio/AudioEngine.hpp"
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>


/**
 * @file AudioEngine.cpp
 * @brief Implements AudioEngine: device management, audio callback, and output mapping.
 *
 * Only implementation details and non-trivial logic are commented here, as API documentation is in the header.
 */
namespace ms {

AudioEngine::AudioEngine(GraphManager* graph)
  : graph_(graph), 
    blockSize_(512), 
    numOutputChannels_(2), 
    numInputChannels_(0),
    fadeOutDurationMs_(0.0f),
    fadeOutSamples_(0),
    currentFadeSample_(0),
    fadeOutActive_(false) {
  std::memset(&device_, 0, sizeof(device_));
}

AudioEngine::~AudioEngine() {
  stop();
}

/**
 * @brief Audio device callback for real-time streaming.
 *
 * Handles input injection, graph processing, output mapping, and fade-out envelope.
 * @param pDevice Pointer to miniaudio device
 * @param pOutput Output buffer
 * @param pInput Input buffer
 * @param frameCount Number of frames to process
 */
void AudioEngine::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  AudioEngine* engine = (AudioEngine*)pDevice->pUserData;
  if (!engine || !engine->graph_) return;

  float* output = (float*)pOutput;
  const float* input = (const float*)pInput;
  
  // Clear output buffer
  std::memset(output, 0, frameCount * engine->numOutputChannels_ * sizeof(float));
  
  // Inject physical input data into GraphManager (before processing)
  if (input && engine->numInputChannels_ > 0) {
    // Temporary buffer for deinterleaved input
    std::vector<float> deinterleavedInput(frameCount);
    
    // Deinterleave and set each physical input channel
    for (int ch = 0; ch < engine->numInputChannels_; ++ch) {
      // Deinterleave this channel
      for (ma_uint32 i = 0; i < frameCount; ++i) {
        deinterleavedInput[i] = input[i * engine->numInputChannels_ + ch];
      }
      
      // Set in GraphManager - any node can now access it
      engine->graph_->setPhysicalInput(ch, deinterleavedInput.data(), frameCount);
    }
  }
  
  // Process graph
  engine->graph_->process(frameCount);
  
  // Map graph outputs to physical channels
  for (int ch = 0; ch < engine->numOutputChannels_ && ch < (int)engine->outputChannelMappings_.size(); ++ch) {
    const auto& mapping = engine->outputChannelMappings_[ch];
    const float* graphOutput = engine->graph_->getNodeOutput(mapping.nodeId, mapping.portIndex);
    
    if (graphOutput) {
      // Interleave channel data
      for (ma_uint32 i = 0; i < frameCount; ++i) {
        output[i * engine->numOutputChannels_ + ch] = graphOutput[i];
      }
    }
  }
  
  // Apply fade-out if active
  if (engine->fadeOutActive_) {
    for (ma_uint32 i = 0; i < frameCount; ++i) {
      if (engine->currentFadeSample_ < engine->fadeOutSamples_) {
        // Calculate fade-out gain (1.0 -> 0.0)
        float fadeGain = 1.0f - (static_cast<float>(engine->currentFadeSample_) / engine->fadeOutSamples_);
        
        // Apply to all channels
        for (int ch = 0; ch < engine->numOutputChannels_; ++ch) {
          output[i * engine->numOutputChannels_ + ch] *= fadeGain;
        }
        
        engine->currentFadeSample_++;
      } else {
        // Fade-out complete, mute remaining samples
        for (ma_uint32 j = i; j < frameCount; ++j) {
          for (int ch = 0; ch < engine->numOutputChannels_; ++ch) {
            output[j * engine->numOutputChannels_ + ch] = 0.0f;
          }
        }
        break;
      }
    }
  }
}

/**
 * @brief Initialize and start the audio device, preparing the graph and setting up streaming.
 */
bool AudioEngine::start(int sampleRate, int blockSize, int numOutputChannels, int numInputChannels) {
  blockSize_ = blockSize;
  numOutputChannels_ = numOutputChannels;
  numInputChannels_ = numInputChannels;
  
  ma_device_type deviceType = ma_device_type_playback;
  if (numInputChannels_ > 0) {
    deviceType = ma_device_type_duplex; // Both input and output
  }
  
  ma_device_config deviceConfig = ma_device_config_init(deviceType);
  deviceConfig.playback.format = ma_format_f32;
  deviceConfig.playback.channels = numOutputChannels_;
  deviceConfig.capture.format = ma_format_f32;
  deviceConfig.capture.channels = numInputChannels_;
  deviceConfig.sampleRate = sampleRate;
  deviceConfig.dataCallback = AudioEngine::audioCallback;
  deviceConfig.pUserData = this;

  if (ma_device_init(NULL, &deviceConfig, &device_) != MA_SUCCESS) {
    std::cerr << "Failed to initialize audio device." << std::endl;
    return false;
  }

  graph_->prepare(device_.sampleRate, blockSize_);
  
  // Reset fade-out state when starting
  fadeOutActive_ = false;
  currentFadeSample_ = 0;
  fadeOutSamples_ = 0;

  if (ma_device_start(&device_) != MA_SUCCESS) {
    std::cerr << "Failed to start audio device." << std::endl;
    ma_device_uninit(&device_);
    return false;
  }

  std::cout << "Audio device started:" << std::endl;
  std::cout << "  Sample rate: " << device_.sampleRate << " Hz" << std::endl;
  std::cout << "  Output channels: " << numOutputChannels_ << std::endl;
  std::cout << "  Input channels: " << numInputChannels_ << std::endl;

  return true;
}

/**
 * @brief Stop the audio device, optionally applying a fade-out envelope.
 */
void AudioEngine::stop(float fadeOutMs) {
  // Use default fade-out duration if not specified
  if (fadeOutMs == 0.0f && fadeOutDurationMs_ > 0.0f) {
    fadeOutMs = fadeOutDurationMs_;
  }
  
  if (fadeOutMs > 0.0f) {
    // Calculate fade-out samples
    fadeOutSamples_ = static_cast<int>((fadeOutMs / 1000.0f) * device_.sampleRate);
    currentFadeSample_ = 0;
    fadeOutActive_ = true;
    
    std::cout << "Fading out over " << fadeOutMs << " ms (" << fadeOutSamples_ << " samples)..." << std::endl;
    
    // Wait for fade-out to complete
    // Sleep slightly longer to ensure all samples are processed
    int sleepMs = static_cast<int>(fadeOutMs) + 100;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
  }
  
  ma_device_uninit(&device_);
  fadeOutActive_ = false;
}

/**
 * @brief Map a graph node output to a physical output channel for playback.
 */
void AudioEngine::mapOutputChannel(int channelIndex, const std::string& nodeId, int outputIndex) {
  if (channelIndex >= (int)outputChannelMappings_.size()) {
    outputChannelMappings_.resize(channelIndex + 1);
  }
  outputChannelMappings_[channelIndex] = {nodeId, outputIndex};
}

/**
 * @brief Clear all output channel mappings.
 */
void AudioEngine::clearOutputChannelMappings() {
  outputChannelMappings_.clear();
}

} // namespace ms
