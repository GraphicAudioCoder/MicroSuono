#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/nodes/MonoToStereoNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

int main() {
  std::cout << "=== MicroSuono Multichannel Demo ===" << std::endl << std::endl;

  // Example 1: Direct stereo routing (two independent mono channels)
  std::cout << "Example 1: Direct stereo routing" << std::endl;
  {
    ms::GraphManager graph;

    auto sine1 = std::make_shared<ms::SineNode>("sine1", 440.0f);
    auto sine2 = std::make_shared<ms::SineNode>("sine2", 554.37f);
    auto gain1 = std::make_shared<ms::GainNode>("gain1", 0.2f);
    auto gain2 = std::make_shared<ms::GainNode>("gain2", 0.2f);

    graph.createNode("sine1", sine1);
    graph.createNode("sine2", sine2);
    graph.createNode("gain1", gain1);
    graph.createNode("gain2", gain2);
    
    graph.connect("sine1", "out", "gain1", "in");
    graph.connect("sine2", "out", "gain2", "in");

    ms::AudioEngine audio(&graph);
    audio.start(44100, 512, 2);
    audio.mapOutputChannel(0, "gain1", 0);
    audio.mapOutputChannel(1, "gain2", 0);

    std::cout << "  Left:  440.00 Hz (A4)" << std::endl;
    std::cout << "  Right: 554.37 Hz (C#5)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    audio.stop();
  }

  std::cout << std::endl;

  // Example 2: Mono to stereo with panning
  std::cout << "Example 2: Mono to stereo with center panning" << std::endl;
  {
    ms::GraphManager graph;

    auto sine = std::make_shared<ms::SineNode>("sine", 440.0f);
    auto gain = std::make_shared<ms::GainNode>("gain", 0.2f);
    auto stereo = std::make_shared<ms::MonoToStereoNode>("stereo", 0.0f); // Center pan

    graph.createNode("sine", sine);
    graph.createNode("gain", gain);
    graph.createNode("stereo", stereo);
    
    graph.connect("sine", "out", "gain", "in");
    graph.connect("gain", "out", "stereo", "in");

    ms::AudioEngine audio(&graph);
    audio.start(44100, 512, 2);
    audio.mapOutputChannel(0, "stereo", 0);  // Left output
    audio.mapOutputChannel(1, "stereo", 1);  // Right output

    std::cout << "  Mono 440 Hz → Center pan → Stereo" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    audio.stop();
  }

  std::cout << std::endl << "Done!" << std::endl;
  return 0;
}
