# MonoToStereoNode - Mono to Stereo Converter with Panning

## Purpose

Converts a mono audio signal into stereo output with controllable pan position.

## Ports

### Inputs
- `in` (Audio): Mono audio input signal
- `pan` (Control): Pan position
  - `-1.0` = full left
  - `0.0` = center (equal L/R)
  - `+1.0` = full right

### Outputs
- `left` (Audio): Left channel output
- `right` (Audio): Right channel output

## Panning Algorithm

Uses **constant power panning** to maintain equal loudness across the stereo field:

```
angle = (pan + 1.0) * π/4        // Map -1..+1 to 0..π/2
leftGain = cos(angle)
rightGain = sin(angle)
```

This ensures that when panned to center, both channels have gain ≈ 0.707 (-3dB), maintaining perceived loudness.

## Usage Example

### Center Pan (default)

```cpp
auto sine = std::make_shared<SineNode>("sine", 440.0f);
auto stereo = std::make_shared<MonoToStereoNode>("stereo", 0.0f);

graph.createNode("sine", sine);
graph.createNode("stereo", stereo);
graph.connect("sine", "out", "stereo", "in");

audio.mapOutputChannel(0, "stereo", 0);  // Left output
audio.mapOutputChannel(1, "stereo", 1);  // Right output (port index 1)
```

### Dynamic Panning

```cpp
auto stereo = std::make_shared<MonoToStereoNode>("stereo");
auto lfo = std::make_shared<SineNode>("lfo", 0.5f);  // 0.5 Hz LFO

graph.createNode("stereo", stereo);
graph.createNode("lfo", lfo);
graph.connect("lfo", "out", "stereo", "pan");  // Modulate pan position
```

## Multichannel Routing

MonoToStereoNode has **two separate output ports** (`left` and `right`), not one stereo port.

Each output must be mapped to a physical channel:

```cpp
// ✓ Correct: Map each output port to a channel
audio.mapOutputChannel(0, "stereo", 0);  // Left (port 0)
audio.mapOutputChannel(1, "stereo", 1);  // Right (port 1)

// ✗ Wrong: Only maps left channel
audio.mapOutputChannel(0, "stereo", 0);
```

## Pan Law

| Pan Value | Left Gain | Right Gain |
|-----------|-----------|------------|
| -1.0      | 1.0       | 0.0        |
| -0.5      | 0.92      | 0.38       |
| 0.0       | 0.707     | 0.707      |
| +0.5      | 0.38      | 0.92       |
| +1.0      | 0.0       | 1.0        |

