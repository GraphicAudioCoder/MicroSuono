#include "MicroSuono/nodes/SliderNode.hpp"
#include <algorithm>

namespace ms {

SliderNode::SliderNode(const std::string& id, 
                       float minValue, 
                       float maxValue,
                       float defaultValue,
                       ScaleType scale)
  : Node(id)
  , minValue_(minValue)
  , maxValue_(maxValue)
  , scaleType_(scale)
{
  // Clamp default value to range
  value_ = std::clamp(defaultValue, minValue_, maxValue_);
  
  // Add output port
  addOutputPort("value", PortType::Control);
}

void SliderNode::prepare(int sampleRate, int blockSize) {
  Node::prepare(sampleRate, blockSize);
}

void SliderNode::process(const float* const* audioInputs, float** audioOutputs, int nFrames) {
  // SliderNode doesn't process audio
}

void SliderNode::processControl(
  const std::unordered_map<std::string, ControlValue>& controlInputs,
  std::unordered_map<std::string, ControlValue>& controlOutputs)
{
  // Output current value on the "value" port
  controlOutputs["value"] = value_;
}

void SliderNode::setValue(float value) {
  value_ = std::clamp(value, minValue_, maxValue_);
}

void SliderNode::setValueNormalized(float normalized) {
  normalized = std::clamp(normalized, 0.0f, 1.0f);
  value_ = normalizedToValue(normalized);
}

float SliderNode::getValueNormalized() const {
  return valueToNormalized(value_);
}

void SliderNode::setRange(float minValue, float maxValue) {
  minValue_ = minValue;
  maxValue_ = maxValue;
  // Re-clamp current value
  value_ = std::clamp(value_, minValue_, maxValue_);
}

float SliderNode::normalizedToValue(float normalized) const {
  if (scaleType_ == ScaleType::Logarithmic) {
    // Logarithmic scaling: value = min * (max/min)^normalized
    if (minValue_ <= 0.0f) {
      // Can't do log scale with non-positive min, fall back to linear
      return minValue_ + normalized * (maxValue_ - minValue_);
    }
    return minValue_ * std::pow(maxValue_ / minValue_, normalized);
  } else {
    // Linear scaling
    return minValue_ + normalized * (maxValue_ - minValue_);
  }
}

float SliderNode::valueToNormalized(float value) const {
  if (scaleType_ == ScaleType::Logarithmic) {
    if (minValue_ <= 0.0f || value <= 0.0f) {
      // Fall back to linear
      return (value - minValue_) / (maxValue_ - minValue_);
    }
    return std::log(value / minValue_) / std::log(maxValue_ / minValue_);
  } else {
    // Linear scaling
    return (value - minValue_) / (maxValue_ - minValue_);
  }
}

} // namespace ms
