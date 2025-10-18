// Demo: Mixer with multiple sine wave inputs
// Demonstrates array/vector support for multiple inputs
#include <MicroSuono/GraphManager.hpp>
#include <MicroSuono/audio/AudioEngine.hpp>
#include <nodes/SineNode.hpp>
#include <nodes/MixerNode.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  std::cout << "=== MicroSuono Demo: Mixer with Vector Inputs ===" << std::endl;
  std::cout << "Creates 4 sine waves at different frequencies and mixes them" << std::endl;
  
  ms::GraphManager graph;
  
  // Create 4 sine oscillators with different frequencies
  auto sine1 = std::make_shared<ms::SineNode>("sine1");
  auto sine2 = std::make_shared<ms::SineNode>("sine2");
  auto sine3 = std::make_shared<ms::SineNode>("sine3");
  auto sine4 = std::make_shared<ms::SineNode>("sine4");
  
  // Create mixer with 4 inputs, stereo output
  auto mixer = std::make_shared<ms::MixerNode>("mixer", 4, true);
  
  // Add nodes to graph
  graph.createNode("sine1", sine1);
  graph.createNode("sine2", sine2);
  graph.createNode("sine3", sine3);
  graph.createNode("sine4", sine4);
  graph.createNode("mixer", mixer);
  
  // Connect sines to mixer inputs (array-like connections)
  graph.connect("sine1", "out", "mixer", "in_0");
  graph.connect("sine2", "out", "mixer", "in_1");
  graph.connect("sine3", "out", "mixer", "in_2");
  graph.connect("sine4", "out", "mixer", "in_3");
  
  // Initialize audio engine
  ms::AudioEngine audio(&graph);
  audio.setFadeOutDuration(500.0f);
  audio.start(44100, 512, 2, 0);  // 2 outputs, 0 inputs
  
  // Connect mixer to physical outputs
  audio.mapOutputChannel(0, "mixer", 0);  // Left channel
  audio.mapOutputChannel(1, "mixer", 1);  // Right channel
  
  std::cout << "\nAudio engine initialized:" << std::endl;
  std::cout << "  Mixer inputs: " << mixer->getNumInputs() << std::endl;
  std::cout << "  Stereo output: " << (mixer->isStereo() ? "Yes" : "No") << std::endl;
  
  std::cout << "\n🎵 Playing 4-voice chord (C major) - Centered..." << std::endl;
  
  // Set frequencies for C major chord + octave
  sine1->setFrequency(261.63f);  // C4
  sine2->setFrequency(329.63f);  // E4
  sine3->setFrequency(392.00f);  // G4
  sine4->setFrequency(523.25f);  // C5
  
  // Set individual channel gains (create a mix)
  mixer->setChannelGain(0, 0.3f);  // C4 - fundamental
  mixer->setChannelGain(1, 0.25f); // E4 - third
  mixer->setChannelGain(2, 0.2f);  // G4 - fifth
  mixer->setChannelGain(3, 0.15f); // C5 - octave (quieter)
  
  // All channels centered by default (pan = 0.0)
  // This gives a mono-like centered sound
  
  // Set master gain
  mixer->setMasterGain(0.6f);
  
  std::cout << "\nChannel setup (all centered):" << std::endl;
  std::cout << "  Ch 0 (C4 261Hz): 30% gain, Pan: CENTER" << std::endl;
  std::cout << "  Ch 1 (E4 329Hz): 25% gain, Pan: CENTER" << std::endl;
  std::cout << "  Ch 2 (G4 392Hz): 20% gain, Pan: CENTER" << std::endl;
  std::cout << "  Ch 3 (C5 523Hz): 15% gain, Pan: CENTER" << std::endl;
  std::cout << "  Master gain: 60%" << std::endl;
  
  // Play for 2 seconds
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  std::cout << "\n🎚️  Spreading stereo - panning channels..." << std::endl;
  mixer->setChannelPan(0, -0.7f);  // C4 left
  mixer->setChannelPan(1, -0.3f);  // E4 slight left
  mixer->setChannelPan(2, 0.3f);   // G4 slight right
  mixer->setChannelPan(3, 0.7f);   // C5 right
  
  std::cout << "  Ch 0: LEFT (-0.7)" << std::endl;
  std::cout << "  Ch 1: SLIGHT LEFT (-0.3)" << std::endl;
  std::cout << "  Ch 2: SLIGHT RIGHT (0.3)" << std::endl;
  std::cout << "  Ch 3: RIGHT (0.7)" << std::endl;
  std::cout << "  Ch 0: LEFT (-0.7)" << std::endl;
  std::cout << "  Ch 1: SLIGHT LEFT (-0.3)" << std::endl;
  std::cout << "  Ch 2: SLIGHT RIGHT (0.3)" << std::endl;
  std::cout << "  Ch 3: RIGHT (0.7)" << std::endl;
  
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  std::cout << "\n🎚️  Back to center..." << std::endl;
  mixer->setChannelPan(0, 0.0f);
  mixer->setChannelPan(1, 0.0f);
  mixer->setChannelPan(2, 0.0f);
  mixer->setChannelPan(3, 0.0f);
  
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  std::cout << "\n🎚️  Adjusting mix - emphasizing the octave..." << std::endl;
  mixer->setChannelGain(0, 0.2f);
  mixer->setChannelGain(3, 0.4f);
  
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  std::cout << "\n🎚️  Fading out channels one by one..." << std::endl;
  mixer->setChannelGain(3, 0.0f);
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  
  mixer->setChannelGain(2, 0.0f);
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  
  mixer->setChannelGain(1, 0.0f);
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  
  mixer->setChannelGain(0, 0.0f);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  
  audio.stop();
  std::cout << "\n✓ Demo completed!" << std::endl;
  
  return 0;
}
