#pragma once
#include "../Node.hpp"
#include <cmath>

namespace ms {

/** Slider control node that outputs a controllable value
 * 
 * This node represents a virtual slider/parameter control that can output
 * values in a configurable range with linear or logarithmic scaling.
 * The value can be changed at runtime via setValue() or setValueNormalized().
 * 
 * Output ports:
 * - "value" (Control): Current slider value
 */
class SliderNode : public Node {
public:
  enum class ScaleType {
    Linear,      // Linear mapping between min and max
    Logarithmic  // Logarithmic mapping (useful for frequency, gain in dB)
  };

  /** Construct a slider node
   * @param id Unique node identifier
   * @param minValue Minimum output value
   * @param maxValue Maximum output value
   * @param defaultValue Initial value (will be clamped to range)
   * @param scale Scaling type (Linear or Logarithmic)
   */
  SliderNode(const std::string& id, 
             float minValue = 0.0f, 
             float maxValue = 1.0f,
             float defaultValue = 0.5f,
             ScaleType scale = ScaleType::Linear);
  
  void prepare(int sampleRate, int blockSize) override;
  
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;
  
  /** Set the slider value directly (will be clamped to min/max range)
   * @param value New value
   */
  void setValue(float value);
  
  /** Get current slider value */
  float getValue() const { return value_; }
  
  /** Set value using normalized 0-1 range (applies scaling)
   * @param normalized Value in [0, 1] range
   */
  void setValueNormalized(float normalized);
  
  /** Get normalized value in [0, 1] range */
  float getValueNormalized() const;
  
  /** Set the range for the slider
   * @param minValue New minimum value
   * @param maxValue New maximum value
   */
  void setRange(float minValue, float maxValue);
  
  /** Get minimum value */
  float getMinValue() const { return minValue_; }
  
  /** Get maximum value */
  float getMaxValue() const { return maxValue_; }

private:
  float value_;            ///< Current slider value
  float minValue_;         ///< Minimum value
  float maxValue_;         ///< Maximum value
  ScaleType scaleType_;    ///< Scaling type (Linear or Logarithmic)
  
  // Helper functions for logarithmic scaling
  float normalizedToValue(float normalized) const;
  float valueToNormalized(float value) const;
};

} // namespace ms
