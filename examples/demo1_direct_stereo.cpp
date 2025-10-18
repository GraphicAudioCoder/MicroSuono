#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo1_directStereo() {
  std::cout << "\n=== Demo 1: Direct Stereo Routing ===" << std::endl;
  std::cout << "Two independent mono signals → Left and Right channels\n" << std::endl;

  ms::GraphManager graph;

  auto sine1 = std::make_shared<ms::SineNode>("sine1", 440.0f);
  auto sine2 = std::make_shared<ms::SineNode>("sine2", 554.37f);
  auto gain1 = std::make_shared<ms::GainNode>("gain1", 0.2f);
  auto gain2 = std::make_shared<ms::GainNode>("gain2", 0.2f);

  // Configure fade-in for each node (optional, default is 50ms)
  sine1->setFadeInDuration(500.0f);  // Fast 30ms fade for oscillators
  sine2->setFadeInDuration(500.0f);
  gain1->setFadeInDuration(0.0f);   // No fade-in for gain (input already fades)
  gain2->setFadeInDuration(0.0f);

  graph.createNode("sine1", sine1);
  graph.createNode("sine2", sine2);
  graph.createNode("gain1", gain1);
  graph.createNode("gain2", gain2);
  
  graph.connect("sine1", "out", "gain1", "in");
  graph.connect("sine2", "out", "gain2", "in");

  ms::AudioEngine audio(&graph);
  audio.setFadeOutDuration(500.0f);  // 500ms fade-out on stop
  audio.start(44100, 512, 2, 0);  // 2 outputs, 0 inputs
  audio.mapOutputChannel(0, "gain1", 0);
  audio.mapOutputChannel(1, "gain2", 0);

  std::cout << "  Left:  440.00 Hz (A4)" << std::endl;
  std::cout << "  Right: 554.37 Hz (C#5)" << std::endl;
  std::cout << "  Node fade-in: 30ms (oscillators), 0ms (gain)" << std::endl;
  std::cout << "  Engine fade-out: 500ms" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  audio.stop();  // Will use the configured 500ms fade-out
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║      Demo 1: Direct Stereo Routing     ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo1_directStereo();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
