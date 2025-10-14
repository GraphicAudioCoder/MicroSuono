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

/** Port descriptor */
struct Port {
  std::string name;
  PortType type;
  
  Port(const std::string& name, PortType type) 
    : name(name), type(type) {}
};

} // namespace ms
