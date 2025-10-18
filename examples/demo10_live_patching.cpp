/**
 * Demo 10: Live Patching Test (Interactive)
 * 
 * Simulates Pure Data-style live patching:
 * - Add/remove effects while playing
 * - Reroute signals dynamically
 * - No audio interruptions
 */

#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace ms;

void printMenu() {
  std::cout << "\n┌─────────────────────────────────────┐\n";
  std::cout << "│   Live Patching Test Menu           │\n";
  std::cout << "├─────────────────────────────────────┤\n";
  std::cout << "│ 1. Toggle sine connection           │\n";
  std::cout << "│ 2. Add gain effect                  │\n";
  std::cout << "│ 3. Remove gain effect               │\n";
  std::cout << "│ 4. Change sine frequency            │\n";
  std::cout << "│ 5. Add second oscillator            │\n";
  std::cout << "│ 0. Quit                             │\n";
  std::cout << "└─────────────────────────────────────┘\n";
  std::cout << "Choice: ";
}

int main() {
  std::cout << "\n=== Demo 10: Live Patching (Interactive) ===\n";
  std::cout << "Add/remove nodes and connections while audio is running\n";
  
  GraphManager graph;
  AudioEngine audio(&graph);
  
  // Start with just a sine wave
  auto sine = std::make_shared<SineNode>("sine", 440.0f, 0.4f);
  graph.createNode("sine", sine);
  
  graph.prepare(44100, 512);
  audio.start(44100, 512, 2, 0);
  
  std::cout << "\n🎵 Audio started: Sine wave at 440 Hz\n";
  std::cout << "    Current routing: sine -> [disconnected]\n";
  std::cout << "    Status: Silence (no output connection yet)\n";
  
  bool sineConnected = false;
  bool gainExists = false;
  int numOscillators = 1;
  
  while (true) {
    printMenu();
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
      case 0:
        std::cout << "\n👋 Stopping audio...\n";
        audio.stop();
        return 0;
        
      case 1: {
        // Toggle sine connection
        if (!sineConnected) {
          // No gain node, so this won't produce sound yet
          std::cout << "\n⚠️  No destination available to connect to\n";
          std::cout << "    Try adding a gain effect first (option 2)\n";
        } else {
          if (gainExists) {
            graph.disconnect("sine", "out", "gain", "in");
            std::cout << "\n🔌 Disconnected: sine -> gain\n";
            std::cout << "    Status: Silence\n";
            sineConnected = false;
          }
        }
        break;
      }
      
      case 2: {
        // Add gain effect
        if (!gainExists) {
          auto gain = std::make_shared<GainNode>("gain", 0.8f);
          graph.createNode("gain", gain);
          
          // Connect sine -> gain automatically
          graph.connect("sine", "out", "gain", "in");
          
          // Map gain to output channels
          audio.mapOutputChannel(0, "gain", 0);
          audio.mapOutputChannel(1, "gain", 0);
          
          sineConnected = true;
          gainExists = true;
          
          std::cout << "\n✨ Added gain node (80% gain)\n";
          std::cout << "    New routing: sine -> gain -> output\n";
          std::cout << "    Status: 🔊 Sound ON!\n";
        } else {
          std::cout << "\n⚠️  Gain node already exists\n";
        }
        break;
      }
      
      case 3: {
        // Remove gain effect
        if (gainExists) {
          // Unmap output first
          audio.mapOutputChannel(0, "", 0);  // Clear mapping
          audio.mapOutputChannel(1, "", 0);
          
          graph.removeNode("gain");
          gainExists = false;
          sineConnected = false;
          
          std::cout << "\n🗑️  Removed gain node\n";
          std::cout << "    New routing: sine -> [disconnected]\n";
          std::cout << "    Status: Silence\n";
        } else {
          std::cout << "\n⚠️  No gain node to remove\n";
        }
        break;
      }
      
      case 4: {
        // Change frequency
        std::cout << "\nEnter new frequency (Hz): ";
        float freq;
        std::cin >> freq;
        
        if (freq > 0 && freq < 20000) {
          sine->setFrequency(freq);
          std::cout << "✓ Frequency changed to " << freq << " Hz\n";
          
          if (!sineConnected) {
            std::cout << "  (Still silent - needs connection to output)\n";
          }
        } else {
          std::cout << "⚠️  Invalid frequency (must be 0-20000 Hz)\n";
        }
        break;
      }
      
      case 5: {
        // Add second oscillator
        numOscillators++;
        std::string id = "sine" + std::to_string(numOscillators);
        
        auto newSine = std::make_shared<SineNode>(id, 330.0f, 0.3f);
        graph.createNode(id, newSine);
        
        if (gainExists) {
          graph.connect(id, "out", "gain", "in");
          std::cout << "\n✨ Added oscillator: " << id << " (330 Hz)\n";
          std::cout << "    Connected to gain (mixing with existing signal)\n";
          std::cout << "    Status: 🔊 Hearing chord!\n";
        } else {
          std::cout << "\n✨ Added oscillator: " << id << " (330 Hz)\n";
          std::cout << "    Not connected (need gain node first)\n";
        }
        break;
      }
      
      default:
        std::cout << "\n⚠️  Invalid choice\n";
        break;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  return 0;
}
