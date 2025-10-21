#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/nodes/SliderNode.hpp"
#include "MicroSuono/nodes/SineOscillatorNode.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <atomic>

// Atomic flag to control the audio loop
std::atomic<bool> running(true);

void printHelp() {
  std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║           Interactive Slider Control Demo                ║" << std::endl;
  std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;
  std::cout << "\nCommands:" << std::endl;
  std::cout << "  f <value>  - Set frequency (20-2000 Hz, e.g., 'f 440')" << std::endl;
  std::cout << "  v <value>  - Set volume (0.0-1.0, e.g., 'v 0.5')" << std::endl;
  std::cout << "  s          - Show current values" << std::endl;
  std::cout << "  h          - Show this help" << std::endl;
  std::cout << "  q          - Quit" << std::endl;
  std::cout << "\nType a command and press Enter:\n" << std::endl;
}

void showCurrentValues(const std::shared_ptr<ms::SliderNode>& freqSlider,
                       const std::shared_ptr<ms::SliderNode>& volumeSlider) {
  std::cout << "\n┌─────────────────────────────────┐" << std::endl;
  std::cout << "│ Current Parameter Values        │" << std::endl;
  std::cout << "├─────────────────────────────────┤" << std::endl;
  std::cout << "│ Frequency: " << freqSlider->getValue() << " Hz" << std::string(15 - std::to_string(static_cast<int>(freqSlider->getValue())).length(), ' ') << "│" << std::endl;
  std::cout << "│ Volume:    " << volumeSlider->getValue() << std::string(20 - std::to_string(volumeSlider->getValue()).length(), ' ') << "│" << std::endl;
  std::cout << "└─────────────────────────────────┘\n" << std::endl;
}

void demo11_sliderControl() {
  std::cout << "\nInitializing audio graph..." << std::endl;

  ms::GraphManager graph;

  // Create slider nodes with appropriate ranges and scaling
  auto freqSlider = std::make_shared<ms::SliderNode>(
    "freq_slider", 
    20.0f,      // min: 20 Hz
    2000.0f,    // max: 2000 Hz
    440.0f,     // default: A4
    ms::SliderNode::ScaleType::Logarithmic  // Logarithmic for frequency
  );

  auto volumeSlider = std::make_shared<ms::SliderNode>(
    "volume_slider",
    0.0f,       // min: 0 (silence)
    1.0f,       // max: 1 (full volume)
    0.3f,       // default: 30%
    ms::SliderNode::ScaleType::Linear  // Linear for volume
  );

  // Create sine oscillator with control inputs
  auto oscillator = std::make_shared<ms::SineOscillatorNode>(
    "osc",
    440.0f,     // default frequency
    0.3f        // default amplitude
  );

  // Add nodes to graph
  graph.createNode("freq_slider", freqSlider);
  graph.createNode("volume_slider", volumeSlider);
  graph.createNode("osc", oscillator);

  // Connect control outputs from sliders to oscillator inputs
  graph.connect("freq_slider", "value", "osc", "frequency");
  graph.connect("volume_slider", "value", "osc", "amplitude");

  // Start audio engine
  ms::AudioEngine audio(&graph);
  audio.setFadeOutDuration(200.0f);  // 200ms fade-out on stop
  audio.start(44100, 512, 2, 0);  // Stereo output (same signal on both channels)
  
  // Map oscillator output to both left and right channels
  audio.mapOutputChannel(0, "osc", 0);
  audio.mapOutputChannel(1, "osc", 0);

  std::cout << "✓ Audio engine started!" << std::endl;
  
  printHelp();
  showCurrentValues(freqSlider, volumeSlider);

  // Interactive command loop
  std::string line;
  while (running) {
    std::cout << "> ";
    std::getline(std::cin, line);

    if (line.empty()) continue;

    std::istringstream iss(line);
    char command;
    iss >> command;

    switch (command) {
      case 'f': {
        float freq;
        if (iss >> freq) {
          freqSlider->setValue(freq);
          std::cout << "✓ Frequency set to " << freqSlider->getValue() << " Hz" << std::endl;
        } else {
          std::cout << "✗ Invalid frequency value. Use: f <value>" << std::endl;
        }
        break;
      }
      
      case 'v': {
        float vol;
        if (iss >> vol) {
          volumeSlider->setValue(vol);
          std::cout << "✓ Volume set to " << volumeSlider->getValue() << std::endl;
        } else {
          std::cout << "✗ Invalid volume value. Use: v <value>" << std::endl;
        }
        break;
      }
      
      case 's':
        showCurrentValues(freqSlider, volumeSlider);
        break;
      
      case 'h':
        printHelp();
        break;
      
      case 'q':
        std::cout << "\nStopping audio..." << std::endl;
        running = false;
        break;
      
      default:
        std::cout << "✗ Unknown command. Type 'h' for help." << std::endl;
        break;
    }
  }

  audio.stop();
  std::cout << "✓ Audio stopped." << std::endl;
}

int main() {
  std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║  Demo 11: Interactive Slider Control (Terminal Edition)  ║" << std::endl;
  std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;
  std::cout << "\nThis demo shows how to use SliderNode to control parameters" << std::endl;
  std::cout << "of a SineOscillatorNode in real-time via terminal commands.\n" << std::endl;

  try {
    demo11_sliderControl();
  } catch (const std::exception& e) {
    std::cerr << "✗ Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "\n✓ Demo completed!" << std::endl;
  return 0;
}
