#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  std::cout << "╔════════════════════════════════════════╗" << std::endl;
  std::cout << "║         MicroSuono Test Launcher       ║" << std::endl;
  std::cout << "╚════════════════════════════════════════╝" << std::endl;
  std::cout << "\nUsage: " << argv[0] << " [demo_name]\n" << std::endl;
  std::cout << "Available demos:" << std::endl;
  std::cout << "  - All demos moved to examples/ folder:" << std::endl;
  std::cout << "    • demo1_direct_stereo     - Two independent mono signals" << std::endl;
  std::cout << "    • demo2_mono_to_stereo    - Mono to stereo conversion" << std::endl;
  std::cout << "    • demo3_audio_input       - Microphone passthrough" << std::endl;
  std::cout << "    • demo4_tremolo           - Audio-rate modulation (LFO)" << std::endl;
  std::cout << "    • demo5_summation         - Multiple signals → same input (PD-style)" << std::endl;
  std::cout << "    • audio_input_demo        - Audio input example" << std::endl;
  std::cout << "    • multichannel_demo       - Multichannel routing" << std::endl;
  std::cout << "\nTo run a demo, build and execute it from examples/ folder." << std::endl;
  std::cout << "Example: ./build/demo5_summation\n" << std::endl;
  
  return 0;
}
