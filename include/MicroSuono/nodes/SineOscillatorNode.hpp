#pragma once
#include "../Node.hpp"
#include <cmath>

namespace ms {

/** Sine wave oscillator with control inputs for frequency and amplitude
 * 
 * This node generates a sine wave with controllable frequency and amplitude.
 * Unlike SineNode which uses internal parameters, this node accepts control
 * inputs from other nodes (e.g., SliderNode).
 * 
 * Input ports:
 * - "frequency" (Control): Frequency in Hz (optional, uses default if not connected)
 * - "amplitude" (Control): Amplitude/volume multiplier (optional, uses default if not connected)
 * 
 * Output ports:
 * - "audio_out" (Audio): Generated sine wave signal
 */
class SineOscillatorNode : public Node {
public:
  /** Construct a sine oscillator node
   * @param id Unique node identifier
   * @param defaultFrequency Default frequency when no control input is connected
   * @param defaultAmplitude Default amplitude when no control input is connected
   */
  SineOscillatorNode(const std::string& id, 
                     float defaultFrequency = 440.0f, 
                     float defaultAmplitude = 0.5f);
  
  void prepare(int sampleRate, int blockSize) override;
  
  void process(const float* const* audioInputs, float** audioOutputs, int nFrames) override;
  
  void processControl(
    const std::unordered_map<std::string, ControlValue>& controlInputs,
    std::unordered_map<std::string, ControlValue>& controlOutputs) override;

protected:
  float defaultFrequency_;         ///< Default frequency when no control input
  float defaultAmplitude_;         ///< Default amplitude when no control input
  
  // Current control values (updated in processControl)
  float currentFrequency_;         ///< Current smoothed frequency
  float currentAmplitude_;         ///< Current smoothed amplitude
  
  // Oscillator state
  float phase_;                    ///< Current phase [0, 2π]
  float phaseIncrement_;           ///< Phase increment per sample
  
  // Smoothing for parameter changes (to avoid clicks/pops)
  float targetFrequency_;          ///< Target frequency for smoothing
  float targetAmplitude_;          ///< Target amplitude for smoothing
  float frequencySmoothingCoeff_;  ///< Exponential smoothing coefficient
  float amplitudeSmoothingCoeff_;  ///< Exponential smoothing coefficient
  static constexpr float SMOOTHING_TIME_MS = 10.0f; ///< 10ms smoothing time
  
  void updatePhaseIncrement();
};

} // namespace ms
