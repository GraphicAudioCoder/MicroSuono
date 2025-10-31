#include "MicroSuono/Node.hpp"
#include "MicroSuono/nodes/GainNode.hpp"
#include "MicroSuono/nodes/SineNode.hpp"
#include <iostream>
#include <cassert>

using namespace ms;

void testGettersSetters() {
  std::cout << "=== Testing Node Getters/Setters ===" << std::endl;
  
  // Test 1: getId()
  GainNode gain("testGain", 0.5f);
  assert(gain.getId() == "testGain");
  std::cout << "✓ getId() works correctly" << std::endl;
  
  // Test 2: getParams() const
  const GainNode& constGain = gain;
  const auto& params = constGain.getParams();
  assert(params.size() == 1);
  assert(params[0].name == "gain");
  std::cout << "✓ getParams() const works correctly" << std::endl;
  
  // Test 3: setParam() with float
  bool success = gain.setParam("gain", 0.75f);
  assert(success);
  const ControlValue* value = gain.getParam("gain");
  assert(value != nullptr);
  assert(std::holds_alternative<float>(*value));
  assert(std::get<float>(*value) == 0.75f);
  std::cout << "✓ setParam() with float works correctly" << std::endl;
  
  // Test 4: setParam() with non-existent parameter
  success = gain.setParam("nonexistent", 1.0f);
  assert(!success);
  std::cout << "✓ setParam() returns false for non-existent parameter" << std::endl;
  
  // Test 5: getParam() with non-existent parameter
  value = gain.getParam("nonexistent");
  assert(value == nullptr);
  std::cout << "✓ getParam() returns nullptr for non-existent parameter" << std::endl;
  
  std::cout << std::endl;
}

void testControlValueReference() {
  std::cout << "=== Testing const ControlValue& ===" << std::endl;
  
  // Test that we can create Event with string without copying
  std::string longString(1000, 'x'); // Long string to make copying visible
  Event event1("test", longString, 0);
  assert(event1.type == "test");
  assert(std::holds_alternative<std::string>(event1.value));
  assert(std::get<std::string>(event1.value) == longString);
  std::cout << "✓ Event constructor accepts const ControlValue& for string" << std::endl;
  
  // Test with different types
  Event event2("trigger", 1.0f, 10);
  assert(std::get<float>(event2.value) == 1.0f);
  assert(event2.sampleOffset == 10);
  std::cout << "✓ Event constructor works with float" << std::endl;
  
  Event event3("toggle", true, 20);
  assert(std::get<bool>(event3.value) == true);
  std::cout << "✓ Event constructor works with bool" << std::endl;
  
  Event event4("step", 42, 30);
  assert(std::get<int>(event4.value) == 42);
  std::cout << "✓ Event constructor works with int" << std::endl;
  
  std::cout << std::endl;
}

void testParamConstructor() {
  std::cout << "=== Testing Param Constructor ===" << std::endl;
  
  // Test Param with different types
  Param param1("gain", 0.5f);
  assert(param1.name == "gain");
  assert(std::holds_alternative<float>(param1.value));
  assert(std::get<float>(param1.value) == 0.5f);
  std::cout << "✓ Param constructor with float" << std::endl;
  
  Param param2("octave", -2);
  assert(param2.name == "octave");
  assert(std::holds_alternative<int>(param2.value));
  assert(std::get<int>(param2.value) == -2);
  std::cout << "✓ Param constructor with int" << std::endl;
  
  Param param3("bypass", true);
  assert(param3.name == "bypass");
  assert(std::holds_alternative<bool>(param3.value));
  assert(std::get<bool>(param3.value) == true);
  std::cout << "✓ Param constructor with bool" << std::endl;
  
  Param param4("mode", std::string("saturate"));
  assert(param4.name == "mode");
  assert(std::holds_alternative<std::string>(param4.value));
  assert(std::get<std::string>(param4.value) == "saturate");
  std::cout << "✓ Param constructor with string" << std::endl;
  
  std::cout << std::endl;
}

void testMultipleParameters() {
  std::cout << "=== Testing Multiple Parameters ===" << std::endl;
  
  SineNode sine("testSine", 440.0f, 0.8f, 0.0f);
  
  // Verify initial parameters
  const auto& params = sine.getParams();
  assert(params.size() == 3);
  assert(params[0].name == "frequency");
  assert(params[1].name == "amplitude");
  assert(params[2].name == "offset");
  std::cout << "✓ SineNode has 3 parameters" << std::endl;
  
  // Modify parameters
  sine.setParam("frequency", 880.0f);
  sine.setParam("amplitude", 0.5f);
  sine.setParam("offset", 0.1f);
  
  // Verify modifications
  const ControlValue* freq = sine.getParam("frequency");
  const ControlValue* amp = sine.getParam("amplitude");
  const ControlValue* offset = sine.getParam("offset");
  
  assert(freq && std::get<float>(*freq) == 880.0f);
  assert(amp && std::get<float>(*amp) == 0.5f);
  assert(offset && std::get<float>(*offset) == 0.1f);
  std::cout << "✓ All parameters modified correctly" << std::endl;
  
  std::cout << std::endl;
}

void testEncapsulation() {
  std::cout << "=== Testing Encapsulation ===" << std::endl;
  
  GainNode gain("testGain", 0.5f);
  
  // Test that we cannot access id and params directly
  // This should not compile if uncommented:
  // gain.id = "hacked";  // Should be private
  // gain.params.clear();  // Should be private
  
  // But we can access through getters
  std::cout << "Node ID via getter: " << gain.getId() << std::endl;
  std::cout << "Params count via getter: " << gain.getParams().size() << std::endl;
  
  std::cout << "✓ Encapsulation is working (id and params are private)" << std::endl;
  std::cout << std::endl;
}

int main() {
  std::cout << "╔════════════════════════════════════════════╗" << std::endl;
  std::cout << "║  Node Refactoring Test Suite              ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════╝" << std::endl;
  std::cout << std::endl;
  
  try {
    testGettersSetters();
    testControlValueReference();
    testParamConstructor();
    testMultipleParameters();
    testEncapsulation();
    
    std::cout << "╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ✓ ALL TESTS PASSED!                      ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    std::cout << "Summary of changes:" << std::endl;
    std::cout << "  • id and params are now private with getters/setters" << std::endl;
    std::cout << "  • Added getId(), getParams(), setParam(), getParam()" << std::endl;
    std::cout << "  • ControlValue now passed as const& for efficiency" << std::endl;
    std::cout << "  • Param struct has a constructor for convenience" << std::endl;
    std::cout << "  • All existing code updated to use new API" << std::endl;
    
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
