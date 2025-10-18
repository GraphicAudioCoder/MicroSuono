// Demo: Beating Effect - Natural vs Controlled
// Demonstrates acoustic beating phenomenon and frequency smoothing
#include <MicroSuono/GraphManager.hpp>
#include <MicroSuono/audio/AudioEngine.hpp>
#include <nodes/SineNode.hpp>
#include <nodes/MixerNode.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main() {
  std::cout << "╔═══════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║    MicroSuono Demo: Beating & Vibrato Effect      ║" << std::endl;
  std::cout << "╚═══════════════════════════════════════════════════╝" << std::endl;
  
  ms::GraphManager graph;
  
  // Create two oscillators
  auto sine1 = std::make_shared<ms::SineNode>("sine1");
  auto sine2 = std::make_shared<ms::SineNode>("sine2");
  auto mixer = std::make_shared<ms::MixerNode>("mixer", 2, false); // Mono mixer
  
  graph.createNode("sine1", sine1);
  graph.createNode("sine2", sine2);
  graph.createNode("mixer", mixer);
  
  graph.connect("sine1", "out", "mixer", "in_0");
  graph.connect("sine2", "out", "mixer", "in_1");
  
  ms::AudioEngine audio(&graph);
  audio.start(44100, 512, 1, 0);  // Mono output
  audio.mapOutputChannel(0, "mixer", 0);
  
  mixer->setChannelGain(0, 0.3f);
  mixer->setChannelGain(1, 0.3f);
  
  std::cout << "\n📊 Test 1: NO Beating (Perfect Unison)" << std::endl;
  std::cout << "  Both oscillators at exactly 440.0 Hz" << std::endl;
  sine1->setFrequency(440.0f);
  sine2->setFrequency(440.0f);
  std::cout << "  Expected: Stable, clean tone" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  
  std::cout << "\n📊 Test 2: Slow Beating (2 Hz)" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 442.0 Hz" << std::endl;
  std::cout << "  Difference: 2 Hz → 2 pulses per second" << std::endl;
  sine2->setFrequency(442.0f);
  std::cout << "  Expected: Clear, slow tremolo (wah-wah-wah)" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(4));
  
  std::cout << "\n📊 Test 3: Fast Beating (10 Hz)" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 450.0 Hz" << std::endl;
  std::cout << "  Difference: 10 Hz → 10 pulses per second" << std::endl;
  sine2->setFrequency(450.0f);
  std::cout << "  Expected: Faster tremolo" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  
  std::cout << "\n📊 Test 4: Very Fast Beating (30 Hz)" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 470.0 Hz" << std::endl;
  std::cout << "  Difference: 30 Hz → Perceived as roughness" << std::endl;
  sine2->setFrequency(470.0f);
  std::cout << "  Expected: Rough, grainy sound" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  
  std::cout << "\n📊 Test 5: Perfect Octave (NO Beating)" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 880.0 Hz" << std::endl;
  std::cout << "  Ratio: 2:1 perfect octave" << std::endl;
  sine2->setFrequency(880.0f);
  std::cout << "  Expected: Stable, rich tone" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  
  std::cout << "\n📊 Test 6: Slight Octave Detune" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 881.0 Hz" << std::endl;
  std::cout << "  Almost octave but +1 Hz off" << std::endl;
  sine2->setFrequency(881.0f);
  std::cout << "  Expected: 1 Hz beating (subtle vibrato)" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(4));
  
  std::cout << "\n📊 Test 7: Chorus Effect (Micro-detune)" << std::endl;
  std::cout << "  Sine1: 440.0 Hz, Sine2: 440.0 * 1.005 = 442.2 Hz" << std::endl;
  std::cout << "  Detune: ~5 cents (8.6 cents actual)" << std::endl;
  sine2->setFrequency(440.0f * 1.005f);
  std::cout << "  Expected: Warm 'chorus' effect (~2.2 Hz beating)" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(4));
  
  audio.stop();
  
  std::cout << "\n📊 Test 8: C Major Chord (Multiple Beating)" << std::endl;
  std::cout << "  Creating C major chord with 4 oscillators..." << std::endl;
  std::cout << "  (Restarting audio with 4-voice configuration...)" << std::endl;
  
  // Add more oscillators for full chord
  auto sine3 = std::make_shared<ms::SineNode>("sine3");
  auto sine4 = std::make_shared<ms::SineNode>("sine4");
  auto mixer4 = std::make_shared<ms::MixerNode>("mixer4", 4, false);
  
  graph.createNode("sine3", sine3);
  graph.createNode("sine4", sine4);
  graph.createNode("mixer4", mixer4);
  
  graph.connect("sine1", "out", "mixer4", "in_0");
  graph.connect("sine2", "out", "mixer4", "in_1");
  graph.connect("sine3", "out", "mixer4", "in_2");
  graph.connect("sine4", "out", "mixer4", "in_3");
  
  audio.start(44100, 512, 1, 0);
  audio.mapOutputChannel(0, "mixer4", 0);
  
  mixer4->setChannelGain(0, 0.25f);
  mixer4->setChannelGain(1, 0.20f);
  mixer4->setChannelGain(2, 0.18f);
  mixer4->setChannelGain(3, 0.15f);
  
  float C4 = 261.6256f;
  float E4 = C4 * std::pow(2.0f, 4.0f/12.0f);  // 329.6276 Hz
  float G4 = C4 * std::pow(2.0f, 7.0f/12.0f);  // 391.9954 Hz
  float C5 = C4 * 2.0f;                         // 523.2512 Hz
  
  std::cout << "  C4: " << C4 << " Hz" << std::endl;
  std::cout << "  E4: " << E4 << " Hz" << std::endl;
  std::cout << "  G4: " << G4 << " Hz" << std::endl;
  std::cout << "  C5: " << C5 << " Hz" << std::endl;
  std::cout << "  E4-C4 difference: " << (E4-C4) << " Hz → beating at ~68 Hz" << std::endl;
  std::cout << "  Expected: Complex beating/vibrato effect" << std::endl;
  
  sine1->setFrequency(C4);
  sine2->setFrequency(E4);
  sine3->setFrequency(G4);
  sine4->setFrequency(C5);
  
  std::this_thread::sleep_for(std::chrono::seconds(4));
  
  audio.stop();
  std::cout << "\n✓ Demo completed!" << std::endl;
  std::cout << "\n💡 Key Takeaways:" << std::endl;
  std::cout << "  • Beating = |freq1 - freq2|" << std::endl;
  std::cout << "  • < 15 Hz → Clear tremolo/vibrato" << std::endl;
  std::cout << "  • 15-30 Hz → Roughness" << std::endl;
  std::cout << "  • > 30 Hz → Separate pitch (difference tone)" << std::endl;
  std::cout << "  • Perfectly tuned intervals = No beating" << std::endl;
  std::cout << "  • Slight detune = 'Warm' organic sound" << std::endl;
  
  return 0;
}
