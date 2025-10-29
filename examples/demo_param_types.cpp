#include "MicroSuono/Node.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include <iostream>
#include <iomanip>

using namespace ms;

// Helper function to print ControlValue
void printControlValue(const std::string& name, const ControlValue& value) {
  std::cout << name << " = ";
  
  if (std::holds_alternative<float>(value)) {
    std::cout << std::get<float>(value) << " (float)";
  } else if (std::holds_alternative<int>(value)) {
    std::cout << std::get<int>(value) << " (int)";
  } else if (std::holds_alternative<bool>(value)) {
    std::cout << (std::get<bool>(value) ? "true" : "false") << " (bool)";
  } else if (std::holds_alternative<std::string>(value)) {
    std::cout << "\"" << std::get<std::string>(value) << "\" (string)";
  }
  
  std::cout << std::endl;
}

// Demo node that uses different parameter types
class MultiParamNode : public Node {
public:
  MultiParamNode() : Node("MultiParamDemo") {
    // Add parameters of different types
    params.push_back({"gain", 0.5f});                    // float
    params.push_back({"octave", 0});                     // int
    params.push_back({"bypass", false});                 // bool
    params.push_back({"mode", std::string("normal")});   // string
    
    addInputPort("audio_in", PortType::Audio);
    addOutputPort("audio_out", PortType::Audio);
  }
  
  void process(const float *const *audioInputs, float **audioOutputs, int nFrames) override {
    // Demo processing that uses the parameters
    float gain = std::get<float>(params[0].value);
    int octave = std::get<int>(params[1].value);
    bool bypass = std::get<bool>(params[2].value);
    std::string mode = std::get<std::string>(params[3].value);
    
    if (bypass) {
      // Just copy input to output when bypassed
      for (int i = 0; i < nFrames; ++i) {
        audioOutputs[0][i] = audioInputs[0][i];
      }
    } else {
      // Apply gain and demonstrate we can use all parameter types
      for (int i = 0; i < nFrames; ++i) {
        float sample = audioInputs[0][i];
        
        // Apply gain
        sample *= gain;
        
        // Octave shift could affect pitch (simplified demo)
        // In real implementation, this would use pitch shifting
        if (octave != 0) {
          sample *= (1.0f + octave * 0.1f);
        }
        
        // Mode could change processing algorithm
        if (mode == "saturate") {
          // Simple saturation
          if (sample > 0.8f) sample = 0.8f;
          if (sample < -0.8f) sample = -0.8f;
        }
        
        audioOutputs[0][i] = sample;
      }
    }
  }
};

int main() {
  std::cout << "=== Demo: Node Parameters with ControlValue ===" << std::endl;
  std::cout << std::endl;
  
  // Create a node with multiple parameter types
  MultiParamNode node;
  
  std::cout << "Initial parameters:" << std::endl;
  for (const auto& param : node.params) {
    printControlValue(param.name, param.value);
  }
  std::cout << std::endl;
  
  // Modify parameters
  std::cout << "Modifying parameters..." << std::endl;
  node.params[0].value = 0.75f;                    // Change gain
  node.params[1].value = -2;                       // Change octave
  node.params[2].value = true;                     // Enable bypass
  node.params[3].value = std::string("saturate");  // Change mode
  std::cout << std::endl;
  
  std::cout << "Updated parameters:" << std::endl;
  for (const auto& param : node.params) {
    printControlValue(param.name, param.value);
  }
  std::cout << std::endl;
  
  // Demonstrate type safety
  std::cout << "Type checking example:" << std::endl;
  if (std::holds_alternative<float>(node.params[0].value)) {
    std::cout << "✓ params[0] (gain) is correctly a float" << std::endl;
  }
  if (std::holds_alternative<bool>(node.params[2].value)) {
    std::cout << "✓ params[2] (bypass) is correctly a bool" << std::endl;
  }
  std::cout << std::endl;
  
  // Simulate audio processing
  std::cout << "Simulating audio processing with bypass=" 
            << (std::get<bool>(node.params[2].value) ? "ON" : "OFF") << std::endl;
  
  const int blockSize = 8;
  float inputBuffer[blockSize] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
  float outputBuffer[blockSize];
  
  const float* inputs[1] = {inputBuffer};
  float* outputs[1] = {outputBuffer};
  
  node.prepare(44100, blockSize);
  node.process(inputs, outputs, blockSize);
  
  std::cout << "Input:  ";
  for (int i = 0; i < blockSize; ++i) {
    std::cout << std::fixed << std::setprecision(2) << inputBuffer[i] << " ";
  }
  std::cout << std::endl;
  
  std::cout << "Output: ";
  for (int i = 0; i < blockSize; ++i) {
    std::cout << std::fixed << std::setprecision(2) << outputBuffer[i] << " ";
  }
  std::cout << std::endl;
  std::cout << std::endl;
  
  // Now turn bypass off and try again
  node.params[2].value = false;
  std::cout << "Simulating with bypass=OFF and gain=" 
            << std::get<float>(node.params[0].value) << std::endl;
  
  node.process(inputs, outputs, blockSize);
  
  std::cout << "Output: ";
  for (int i = 0; i < blockSize; ++i) {
    std::cout << std::fixed << std::setprecision(2) << outputBuffer[i] << " ";
  }
  std::cout << std::endl;
  
  std::cout << std::endl;
  std::cout << "✓ Demo completed successfully!" << std::endl;
  
  return 0;
}
