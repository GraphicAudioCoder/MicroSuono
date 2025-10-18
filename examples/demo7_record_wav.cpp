// Demo: Record individual channels and mix to WAV files
// For analyzing beating/vibrato effect
#include <MicroSuono/GraphManager.hpp>
#include <MicroSuono/audio/AudioEngine.hpp>
#include <nodes/SineNode.hpp>
#include <nodes/MixerNode.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>

// Simple WAV file writer
class WavWriter {
private:
  std::ofstream file_;
  uint32_t dataSize_ = 0;
  uint32_t sampleRate_;
  uint16_t numChannels_;
  
  void writeString(const char* str, size_t len) {
    file_.write(str, len);
  }
  
  void writeUInt32(uint32_t value) {
    file_.write(reinterpret_cast<const char*>(&value), 4);
  }
  
  void writeUInt16(uint16_t value) {
    file_.write(reinterpret_cast<const char*>(&value), 2);
  }
  
public:
  WavWriter(const std::string& filename, uint32_t sampleRate, uint16_t numChannels)
    : sampleRate_(sampleRate), numChannels_(numChannels) {
    file_.open(filename, std::ios::binary);
    
    // Write WAV header (will update later with actual size)
    writeString("RIFF", 4);
    writeUInt32(0);  // Placeholder for file size
    writeString("WAVE", 4);
    
    // Format chunk
    writeString("fmt ", 4);
    writeUInt32(16);  // Chunk size
    writeUInt16(1);   // PCM format
    writeUInt16(numChannels_);
    writeUInt32(sampleRate_);
    writeUInt32(sampleRate_ * numChannels_ * 2);  // Byte rate
    writeUInt16(numChannels_ * 2);  // Block align
    writeUInt16(16);  // Bits per sample
    
    // Data chunk header
    writeString("data", 4);
    writeUInt32(0);  // Placeholder for data size
  }
  
  void writeSamples(const float* samples, size_t numSamples) {
    for (size_t i = 0; i < numSamples; ++i) {
      // Convert float [-1, 1] to int16
      float sample = samples[i];
      sample = std::max(-1.0f, std::min(1.0f, sample));  // Clamp
      int16_t intSample = static_cast<int16_t>(sample * 32767.0f);
      file_.write(reinterpret_cast<const char*>(&intSample), 2);
      dataSize_ += 2;
    }
  }
  
  ~WavWriter() {
    // Update file size in header
    uint32_t fileSize = 36 + dataSize_;
    file_.seekp(4);
    writeUInt32(fileSize);
    
    // Update data size
    file_.seekp(40);
    writeUInt32(dataSize_);
    
    file_.close();
  }
};

// Custom processing node to capture and save audio
class RecorderNode : public ms::Node {
private:
  std::vector<float> buffer_;
  size_t maxSamples_;
  size_t currentSample_ = 0;
  
public:
  RecorderNode(const std::string& id, size_t maxSamples)
    : Node(id), maxSamples_(maxSamples) {
    buffer_.resize(maxSamples, 0.0f);
    addInputPort("in", ms::PortType::Audio);
    addOutputPort("out", ms::PortType::Audio);
  }
  
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override {
    if (!audioInputs || !audioInputs[0] || !audioOutputs || !audioOutputs[0]) return;
    
    for (int i = 0; i < nFrames && currentSample_ < maxSamples_; ++i) {
      float sample = audioInputs[0][i];
      buffer_[currentSample_++] = sample;
      audioOutputs[0][i] = sample;  // Pass through
    }
  }
  
  void saveToWav(const std::string& filename, uint32_t sampleRate) {
    WavWriter writer(filename, sampleRate, 1);  // Mono
    writer.writeSamples(buffer_.data(), currentSample_);
    std::cout << "  Saved: " << filename << " (" << currentSample_ << " samples)" << std::endl;
  }
  
  const std::vector<float>& getBuffer() const { return buffer_; }
  size_t getRecordedSamples() const { return currentSample_; }
};

int main() {
  std::cout << "=== Demo 7: Recording Individual Channels and Mix ===" << std::endl;
  
  const int SAMPLE_RATE = 44100;
  const int BLOCK_SIZE = 512;
  const float DURATION_SEC = 4.0f;
  const size_t MAX_SAMPLES = static_cast<size_t>(SAMPLE_RATE * DURATION_SEC);
  
  ms::GraphManager graph;
  
  // Create oscillators
  auto sine1 = std::make_shared<ms::SineNode>("sine1");
  auto sine2 = std::make_shared<ms::SineNode>("sine2");
  auto sine3 = std::make_shared<ms::SineNode>("sine3");
  auto sine4 = std::make_shared<ms::SineNode>("sine4");
  
  // Create recorders for each channel
  auto rec1 = std::make_shared<RecorderNode>("rec1", MAX_SAMPLES);
  auto rec2 = std::make_shared<RecorderNode>("rec2", MAX_SAMPLES);
  auto rec3 = std::make_shared<RecorderNode>("rec3", MAX_SAMPLES);
  auto rec4 = std::make_shared<RecorderNode>("rec4", MAX_SAMPLES);
  
  // Create mixer
  auto mixer = std::make_shared<ms::MixerNode>("mixer", 4, false);  // Mono output
  
  // Recorder for final mix
  auto recMix = std::make_shared<RecorderNode>("recMix", MAX_SAMPLES);
  
  // Add nodes
  graph.createNode("sine1", sine1);
  graph.createNode("sine2", sine2);
  graph.createNode("sine3", sine3);
  graph.createNode("sine4", sine4);
  graph.createNode("rec1", rec1);
  graph.createNode("rec2", rec2);
  graph.createNode("rec3", rec3);
  graph.createNode("rec4", rec4);
  graph.createNode("mixer", mixer);
  graph.createNode("recMix", recMix);
  
  // Connect: sine -> recorder -> mixer
  graph.connect("sine1", "out", "rec1", "in");
  graph.connect("sine2", "out", "rec2", "in");
  graph.connect("sine3", "out", "rec3", "in");
  graph.connect("sine4", "out", "rec4", "in");
  
  graph.connect("rec1", "out", "mixer", "in_0");
  graph.connect("rec2", "out", "mixer", "in_1");
  graph.connect("rec3", "out", "mixer", "in_2");
  graph.connect("rec4", "out", "mixer", "in_3");
  
  graph.connect("mixer", "out", "recMix", "in");
  
  // Initialize audio engine (output to speakers optional)
  ms::AudioEngine audio(&graph);
  audio.start(SAMPLE_RATE, BLOCK_SIZE, 1, 0);
  audio.mapOutputChannel(0, "recMix", 0);
  
  // Set frequencies for C major chord
  std::cout << "\nRecording C major chord:" << std::endl;
  sine1->setFrequency(261.63f);  // C4
  sine2->setFrequency(329.63f);  // E4
  sine3->setFrequency(392.00f);  // G4
  sine4->setFrequency(523.25f);  // C5
  
  std::cout << "  C4: 261.63 Hz" << std::endl;
  std::cout << "  E4: 329.63 Hz (diff from C4: 68 Hz)" << std::endl;
  std::cout << "  G4: 392.00 Hz (diff from E4: 62.37 Hz)" << std::endl;
  std::cout << "  C5: 523.25 Hz (octave of C4)" << std::endl;
  
  // Set gains
  mixer->setChannelGain(0, 0.3f);
  mixer->setChannelGain(1, 0.25f);
  mixer->setChannelGain(2, 0.2f);
  mixer->setChannelGain(3, 0.15f);
  
  std::cout << "\nRecording for " << DURATION_SEC << " seconds..." << std::endl;
  
  // Process offline (without real-time constraints)
  size_t samplesProcessed = 0;
  
  while (samplesProcessed < MAX_SAMPLES) {
    size_t samplesToProcess = std::min(static_cast<size_t>(BLOCK_SIZE), 
                                        MAX_SAMPLES - samplesProcessed);
    graph.process(static_cast<int>(samplesToProcess));
    samplesProcessed += samplesToProcess;
  }
  
  audio.stop();
  
  std::cout << "\n✓ Recording complete!" << std::endl;
  std::cout << "\nSaving WAV files..." << std::endl;
  
  rec1->saveToWav("output/channel1_C4.wav", SAMPLE_RATE);
  rec2->saveToWav("output/channel2_E4.wav", SAMPLE_RATE);
  rec3->saveToWav("output/channel3_G4.wav", SAMPLE_RATE);
  rec4->saveToWav("output/channel4_C5.wav", SAMPLE_RATE);
  recMix->saveToWav("output/mix_cmajor.wav", SAMPLE_RATE);
  
  std::cout << "\n✓ All files saved to output/ directory" << std::endl;
  std::cout << "\nNext: Run Python script to visualize waveforms" << std::endl;
  
  return 0;
}
