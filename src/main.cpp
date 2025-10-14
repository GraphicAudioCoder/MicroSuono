#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/nodes/ThresholdNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

int main() {
  std::cout << "MicroSuono starting..." << std::endl;

  ms::GraphManager graph;

  // Create nodes
  auto sine = std::make_shared<ms::SineNode>("sine", 440.0f);
  auto threshold = std::make_shared<ms::ThresholdNode>("threshold", 0.0f);
  auto gain = std::make_shared<ms::GainNode>("output", 0.2f);

  // Add nodes to graph
  graph.createNode("sine", sine);
  graph.createNode("threshold", threshold);
  graph.createNode("output", gain);
  
  // Connect: sine → threshold → gain
  graph.connect("sine", "out", "threshold", "in");
  graph.connect("threshold", "out", "output", "in");

  ms::AudioEngine audio(&graph);
  if (!audio.start(44100, 512)) {
    std::cerr << "Failed to start audio engine." << std::endl;
    return -1;
  }

  std::cout << "Playing 440Hz sine wave with threshold detector for 3 seconds..." << std::endl;
  std::cout << "Watch for trigger events when signal crosses 0.0 threshold" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));

  audio.stop();
  std::cout << "Done!" << std::endl;
  return 0;
}
