#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/nodes/MonoToStereoNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo2_monoToStereo() {
  std::cout << "\n=== Demo 2: Mono to Stereo with Panning ===" << std::endl;
  std::cout << "Single mono signal → MonoToStereoNode (center pan) → Stereo\n" << std::endl;

  ms::GraphManager graph;

  auto sine = std::make_shared<ms::SineNode>("sine", 440.0f);
  auto gain = std::make_shared<ms::GainNode>("gain", 0.2f);
  auto stereo = std::make_shared<ms::MonoToStereoNode>("stereo", 0.0f);

  graph.createNode("sine", sine);
  graph.createNode("gain", gain);
  graph.createNode("stereo", stereo);
  
  graph.connect("sine", "out", "gain", "in");
  graph.connect("gain", "out", "stereo", "in");

  ms::AudioEngine audio(&graph);
  audio.start(44100, 512, 2, 0);  // 2 outputs, 0 inputs
  audio.setFadeOutDuration(500.0f);  // 500ms fade-out by default
  audio.mapOutputChannel(0, "stereo", 0);  // Left
  audio.mapOutputChannel(1, "stereo", 1);  // Right

  std::cout << "  Both channels: 440 Hz (A4), centered with constant-power panning" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  audio.stop();
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Demo 2: Mono to Stereo with Panning  ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo2_monoToStereo();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
