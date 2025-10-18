#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo6_fadeOut() {
  std::cout << "\n=== Demo 6: Fade-out Test ===" << std::endl;
  std::cout << "Testing configurable fade-out on stop\n" << std::endl;

  ms::GraphManager graph;

  auto sine = std::make_shared<ms::SineNode>("sine", 440.0f);
  auto gain = std::make_shared<ms::GainNode>("gain", 0.2f);

  graph.createNode("sine", sine);
  graph.createNode("gain", gain);
  
  graph.connect("sine", "out", "gain", "in");

  ms::AudioEngine audio(&graph);
  audio.start(44100, 512, 2, 0);
  audio.mapOutputChannel(0, "gain", 0);
  audio.mapOutputChannel(1, "gain", 0);

  std::cout << "\n  Test 1: Stop with NO fade-out (abrupt stop)" << std::endl;
  std::cout << "  Playing 440 Hz for 2 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "  Stopping abruptly (no fade)..." << std::endl;
  audio.stop(0.0f); // No fade-out
  
  std::cout << "\n  Test 2: Stop with 500ms fade-out" << std::endl;
  audio.start(44100, 512, 2, 0);
  audio.mapOutputChannel(0, "gain", 0);
  audio.mapOutputChannel(1, "gain", 0);
  std::cout << "  Playing 440 Hz for 2 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "  Stopping with 500ms fade-out..." << std::endl;
  audio.stop(500.0f); // 500ms fade-out
  
  std::cout << "\n  Test 3: Stop with 1000ms fade-out" << std::endl;
  audio.start(44100, 512, 2, 0);
  audio.mapOutputChannel(0, "gain", 0);
  audio.mapOutputChannel(1, "gain", 0);
  std::cout << "  Playing 440 Hz for 2 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "  Stopping with 1000ms fade-out..." << std::endl;
  audio.stop(1000.0f); // 1000ms fade-out
  
  std::cout << "\n  ✓ Fade-out test completed!" << std::endl;
  std::cout << "  You should have heard:" << std::endl;
  std::cout << "    1. Abrupt stop (click)" << std::endl;
  std::cout << "    2. Smooth 0.5s fade-out" << std::endl;
  std::cout << "    3. Smooth 1.0s fade-out" << std::endl;
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║       Demo 6: Fade-out Test            ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo6_fadeOut();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
