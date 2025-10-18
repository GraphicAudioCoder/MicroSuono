#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

void demo4_tremolo() {
  std::cout << "\n=== Demo 4: Tremolo Effect (Audio-rate Modulation) ===" << std::endl;
  std::cout << "LFO (sine wave) → Gain control → Sine carrier = Tremolo\n" << std::endl;

  ms::GraphManager graph;

  // Carrier signal: 440Hz sine wave at amplitude 0.3
  auto carrier = std::make_shared<ms::SineNode>("carrier", 440.0f, 0.1f, 0.0f);
  
  // LFO: 4Hz sine wave with amplitude 0.2 and offset 0.5
  // This creates a signal that oscillates between 0.3 and 0.7
  auto lfo = std::make_shared<ms::SineNode>("lfo", 4.0f, 0.2f, 0.5f);
  
  // Gain node that will be modulated by the LFO
  auto modulatedGain = std::make_shared<ms::GainNode>("modulated_gain", 0.8f);

  graph.createNode("carrier", carrier);
  graph.createNode("lfo", lfo);
  graph.createNode("modulated_gain", modulatedGain);
  
  // Carrier through modulated gain
  graph.connect("carrier", "out", "modulated_gain", "in");
  
  // LFO modulates the gain (audio->control connection)
  graph.connect("lfo", "out", "modulated_gain", "gain");

  ms::AudioEngine audio(&graph);
  audio.setFadeOutDuration(300.0f);
  audio.start(44100, 512, 2, 0);
  audio.mapOutputChannel(0, "modulated_gain", 0);
  audio.mapOutputChannel(1, "modulated_gain", 0);

  std::cout << "  Carrier: 440 Hz (A4)" << std::endl;
  std::cout << "  LFO: 4 Hz (tremolo rate)" << std::endl;
  std::cout << "  Modulation depth: 0.3 to 0.7 (built into LFO)" << std::endl;
  std::cout << "  Fade-out: 300ms" << std::endl;
  std::cout << "\n  Playing tremolo effect for 4 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(4));
  audio.stop();
  
  std::cout << "\n  ✓ Tremolo effect demonstrated!" << std::endl;
  std::cout << "  Only 3 nodes: carrier + lfo + modulated_gain!" << std::endl;
}

int main() {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║      Demo 4: Tremolo Effect            ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;

  demo4_tremolo();

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
