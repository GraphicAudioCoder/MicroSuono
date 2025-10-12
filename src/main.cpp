#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include <iostream>
#include <memory>
#include <thread>

int main() {
  std::cout << "MicroSuono starting..." << std::endl;

  // Create the GraphManager
  ms::GraphManager graph;

  // Create nodes
  auto sine = std::make_shared<ms::SineNode>("sine1", 440.0f); // A4 440Hz
  auto gain = std::make_shared<ms::GainNode>("gain1", 0.2f);   // Gain at 20%

  // Add nodes to the graph
  graph.createNode("sine1", sine);
  graph.createNode("gain1", gain);

  // Connect sine -> gain
  graph.connect("sine1", 0, "gain1", 0);

  // Create and start the audio engine
  ms::AudioEngine audio(&graph);
  if (!audio.start(44100, 512)) {
    std::cerr << "Failed to start audio engine." << std::endl;
    return -1;
  }

  std::cout << "Starting audio playback..." << std::endl;
  std::cout << "You should hear a 440Hz sine wave (A4 note)" << std::endl;
  std::cout << "Playing for 3 seconds..." << std::endl;

  // Play for 3 seconds
  std::this_thread::sleep_for(std::chrono::seconds(3));

  std::cout << "Stopping..." << std::endl;
  audio.stop();

  std::cout << "Done!" << std::endl;
  return 0;
}
