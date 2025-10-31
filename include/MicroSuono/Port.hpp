#pragma once
#include <string>
#include <variant>

namespace ms {

/** Port data types */
enum class PortType {
  Audio,    // Continuous audio signal (buffer of floats)
  Control,  // Single control value (updated per block)
  Event     // Discrete event/message
};

/** Control value types */
using ControlValue = std::variant<float, int, bool, std::string>;

/** Event message structure */
struct Event {
  std::string type;        // Event type (e.g., "trigger", "noteOn", "bang")
  ControlValue value;      // Optional event data
  int sampleOffset;        // Sample position within block (0 to blockSize-1)
  
  Event(const std::string& type, const ControlValue& value = 0.0f, int sampleOffset = 0)
    : type(type), value(value), sampleOffset(sampleOffset) {}
};

/** Port descriptor */
struct Port {
  std::string name;
  PortType type;
  
  Port(const std::string& name, PortType type) 
    : name(name), type(type) {}
};

} // namespace ms
