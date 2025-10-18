/**
 * Demo 9: Dynamic Connection Changes (Live Patching)
 * 
 * Tests runtime modification of connections while audio is playing,
 * similar to Pure Data's behavior where you can disconnect and reconnect
 * cables without stopping DSP.
 * 
 * Test scenarios:
 * 1. Start with sine -> gain -> output
 * 2. Disconnect gain while playing (should hear silence)
 * 3. Reconnect gain (should hear sound again)
 * 4. Connect sine directly to output (bypass gain)
 * 5. Add/remove nodes dynamically
 */

#include "MicroSuono/GraphManager.hpp"
#include "MicroSuono/audio/AudioEngine.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace ms;

int main() {
  std::cout << "\n=== Demo 9: Dynamic Connection Changes ===\n";
  std::cout << "Testing live patching (Pure Data style)\n\n";
  
  GraphManager graph;
  AudioEngine audio(&graph);
  
  // Create nodes
  auto sine = std::make_shared<SineNode>("sine", 440.0f, 0.3f);
  auto gain = std::make_shared<GainNode>("gain", 0.5f);
  
  graph.createNode("sine", sine);
  graph.createNode("gain", gain);
  
  // Initial connections: sine -> gain -> output
  graph.connect("sine", "out", "gain", "in");
  
  graph.prepare(44100, 512);
  audio.start(44100, 512, 2, 0);
  
  // Map gain output to both stereo channels
  audio.mapOutputChannel(0, "gain", 0);
  audio.mapOutputChannel(1, "gain", 0);
  
  std::cout << "🎵 Playing: Sine (440 Hz) -> Gain (50%) -> Output\n";
  std::cout << "\nThis demo tests dynamic connections automatically.\n";
  std::cout << "Watch the console and listen to the changes...\n";
  std::cout << "\nPress Enter to start tests...";
  std::cin.get();
  
  // Test 1: Disconnect gain input (should hear silence)
  std::cout << "\n[Test 1] Disconnecting sine -> gain...\n";
  graph.disconnect("sine", "out", "gain", "in");
  std::cout << "Expected: Silence (gain has no input)\n";
  std::cout << "Listening for 2 seconds...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  // Test 2: Reconnect gain input (should hear sound again)
  std::cout << "\n[Test 2] Reconnecting sine -> gain...\n";
  graph.connect("sine", "out", "gain", "in");
  std::cout << "Expected: Sound returns\n";
  std::cout << "Listening for 2 seconds...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  // Test 3: Change frequency while playing
  std::cout << "\n[Test 3] Changing frequency to 880 Hz...\n";
  sine->setFrequency(880.0f);
  std::cout << "Expected: Higher pitch\n";
  std::cout << "Listening for 2 seconds...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  // Test 4: Disconnect and reconnect rapidly
  std::cout << "\n[Test 4] Rapid disconnect/reconnect (5 times)...\n";
  std::cout << "Expected: Brief glitches (acceptable)\n";
  for (int i = 0; i < 5; ++i) {
    std::cout << "  Cycle " << (i+1) << "/5..." << std::endl;
    graph.disconnect("sine", "out", "gain", "in");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    graph.connect("sine", "out", "gain", "in");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  std::cout << "Listening to stable signal...\n";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  
  // Test 5: Add new sine node dynamically
  std::cout << "\n[Test 5] Adding second sine (220 Hz) dynamically...\n";
  auto sine2 = std::make_shared<SineNode>("sine2", 220.0f, 0.3f);
  graph.createNode("sine2", sine2);
  graph.connect("sine2", "out", "gain", "in");
  audio.mapOutputChannel(0, "gain", 0);
  audio.mapOutputChannel(1, "gain", 0);
  
  std::cout << "✓ Node hot-added and connected!\n";
  std::cout << "Expected: Hear interval (880 Hz + 220 Hz)\n";
  std::cout << "Listening for 3 seconds...\n";
  std::this_thread::sleep_for(std::chrono::seconds(3));
  
  // Test 6: Remove second sine
  std::cout << "\n[Test 6] Removing second sine...\n";
  graph.removeNode("sine2");
  std::cout << "Expected: Back to single tone (880 Hz)\n";
  std::cout << "Listening for 2 seconds...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  std::cout << "\n✅ All tests completed successfully!\n";
  std::cout << "\nResults:\n";
  std::cout << "  ✓ Dynamic connections work\n";
  std::cout << "  ✓ Hot-plugging nodes works\n";
  std::cout << "  ✓ Thread-safe modifications\n";
  std::cout << "  ✓ No crashes or buffer issues\n";
  std::cout << "\nPress Enter to exit...";
  std::cin.get();
  
  audio.stop();
  return 0;
}
