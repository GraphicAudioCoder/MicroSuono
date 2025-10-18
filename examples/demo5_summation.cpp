#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo5_summation() {
  std::cout << "\n=== Demo 5: Audio Summation (Pure Data style) ===" << std::endl;
  std::cout << "Multiple audio sources → Single input = Automatic summation\n" << std::endl;

  ms::GraphManager graph;

  // Create three sine waves at different frequencies
  auto sine1 = std::make_shared<ms::SineNode>("sine1", 261.63f);  // C4
  auto sine2 = std::make_shared<ms::SineNode>("sine2", 329.63f);  // E4
  auto sine3 = std::make_shared<ms::SineNode>("sine3", 392.00f);  // G4
  auto lfo = std::make_shared<ms::SineNode>("lfo", 4.0f);
  
  // Individual gains for each sine
  auto gain1 = std::make_shared<ms::GainNode>("gain1", 0.15f);
  auto gain2 = std::make_shared<ms::GainNode>("gain2", 0.15f);
  auto gain3 = std::make_shared<ms::GainNode>("gain3", 0.15f);
  
  // Single mixer gain that receives all three signals
  auto mixer = std::make_shared<ms::GainNode>("mixer", 0.9f);
  
  // Output gain
  auto output = std::make_shared<ms::GainNode>("output", 0.5f);

  graph.createNode("sine1", sine1);
  graph.createNode("sine2", sine2);
  graph.createNode("sine3", sine3);
  graph.createNode("gain1", gain1);
  graph.createNode("gain2", gain2);
  graph.createNode("gain3", gain3);
  graph.createNode("mixer", mixer);
  graph.createNode("output", output);
  
  // Connect each sine to its gain
  graph.connect("sine1", "out", "gain1", "in");
  graph.connect("sine2", "out", "gain2", "in");
  graph.connect("sine3", "out", "gain3", "in");
  
  // ALL THREE gains connect to the SAME input of mixer
  // This should SUM the signals (Pure Data style)
  graph.connect("gain1", "out", "mixer", "in");
  graph.connect("gain2", "out", "mixer", "in");
  graph.connect("gain3", "out", "mixer", "in");
  
  // Output
  graph.connect("mixer", "out", "output", "in");

  ms::AudioEngine audio(&graph);
  audio.setFadeOutDuration(300.0f);
  audio.start(44100, 512, 2, 0);
  audio.mapOutputChannel(0, "output", 0);
  audio.mapOutputChannel(1, "output", 0);

  std::cout << "  Three sine waves:" << std::endl;
  std::cout << "    - 261.63 Hz (C4)" << std::endl;
  std::cout << "    - 329.63 Hz (E4)" << std::endl;
  std::cout << "    - 392.00 Hz (G4)" << std::endl;
  std::cout << "  All connected to same mixer input → Automatic summation!" << std::endl;
  std::cout << "  You should hear a C major chord.\n" << std::endl;
  
  std::cout << "  Playing for 3 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  audio.stop();
  
  std::cout << "\n  ✓ Summation test completed!" << std::endl;
  std::cout << "  If you heard a chord (not just the last sine), summation works!" << std::endl;
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║    Demo 5: Audio Summation (PD-style) ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo5_summation();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
