#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/AudioInputNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║   MicroSuono Audio Input Demo         ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;
  std::cout << std::endl;
  std::cout << "This demo captures audio from your microphone" << std::endl;
  std::cout << "and plays it back through your speakers." << std::endl;
  std::cout << "WARNING: This may cause feedback! Lower your volume." << std::endl;
  std::cout << std::endl;

  ms::GraphManager graph;

  // Create audio input node (reads from mic)
  auto micInput = std::make_shared<ms::AudioInputNode>("mic", 0);
  
  // Add gain node to control volume
  auto gain = std::make_shared<ms::GainNode>("gain", 0.5f); // 50% volume

  graph.createNode("mic", micInput);
  graph.createNode("gain", gain);
  
  // Connect: mic → gain → output
  graph.connect("mic", "out", "gain", "in");

  ms::AudioEngine audio(&graph);
  
  // Start with 2 output channels and 1 input channel (mono mic)
  if (!audio.start(44100, 512, 2, 1)) {
    std::cerr << "Failed to start audio engine with input." << std::endl;
    return -1;
  }

  // No registration needed! AudioInputNode automatically reads from physical channel 0
  // The physical input is available to ALL nodes via getPhysicalInput(0)
  
  // Map gain output to both stereo channels
  audio.mapOutputChannel(0, "gain", 0); // Left
  audio.mapOutputChannel(1, "gain", 0); // Right

  std::cout << "Recording and playing back for 5 seconds..." << std::endl;
  std::cout << "(Speak into your microphone)" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  audio.stop();
  std::cout << std::endl << "✓ Done!" << std::endl;
  return 0;
}
