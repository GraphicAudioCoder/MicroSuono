#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/AudioInputNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo3_audioInput() {
  std::cout << "\n=== Demo 3: Audio Input (Microphone Passthrough) ===" << std::endl;
  std::cout << "Physical input → AudioInputNode → Gain → Stereo output\n" << std::endl;
  std::cout << "WARNING: Lower your volume to avoid feedback!\n" << std::endl;

  ms::GraphManager graph;

  // AudioInputNode reads from physical channel 0 automatically
  auto micInput = std::make_shared<ms::AudioInputNode>("mic", 0);
  auto gain = std::make_shared<ms::GainNode>("gain", 0.3f);

  graph.createNode("mic", micInput);
  graph.createNode("gain", gain);
  graph.connect("mic", "out", "gain", "in");

  ms::AudioEngine audio(&graph);
  audio.start(44100, 512, 2, 1);  // 2 outputs, 1 input (mono mic)
  audio.setFadeOutDuration(500.0f);  // 500ms fade-out by default
  audio.mapOutputChannel(0, "gain", 0);  // Left
  audio.mapOutputChannel(1, "gain", 0);  // Right (duplicate mono to stereo)

  std::cout << "  Recording for 3 seconds... (speak into your microphone)" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  audio.stop();
  
  std::cout << "\n  Key point: AudioInputNode uses getPhysicalInput(0) internally" << std::endl;
  std::cout << "  Any custom node can do the same - no special registration!" << std::endl;
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Demo 3: Audio Input (Microphone)     ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo3_audioInput();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
