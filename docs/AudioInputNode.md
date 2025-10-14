# AudioInputNode - Physical Audio Input

## Purpose

Captures audio from physical input devices (microphone, line-in, etc.) and makes it available in the processing graph.

**Key Feature:** Uses the universal `getPhysicalInput()` method - any custom node can do the same!

## Ports

### Outputs
- `out` (Audio): Audio signal from physical input device

## How It Works

AudioInputNode internally calls `getPhysicalInput(channelIndex)` to read from GraphManager's physical input buffers. These buffers are automatically filled by AudioEngine during the audio callback.

**No special registration required!** Just specify which physical channel to read from.

## Usage

### 1. Create AudioInputNode

```cpp
auto micInput = std::make_shared<AudioInputNode>("mic", 0); // Physical channel 0
graph.createNode("mic", micInput);
```

### 2. Configure AudioEngine with Input

```cpp
AudioEngine audio(&graph);
audio.start(44100, 512, 2, 1); // 2 output channels, 1 input channel
```

### 3. Connect and Route

```cpp
// Connect to effects, analysis, or output
graph.connect("mic", "out", "gain", "in");
audio.mapOutputChannel(0, "gain", 0); // Monitor through speakers
```

**That's it!** No registration, no special setup.

## Complete Example: Microphone Passthrough

```cpp
ms::GraphManager graph;

auto micInput = std::make_shared<AudioInputNode>("mic", 0);
auto gain = std::make_shared<GainNode>("gain", 0.5f);

graph.createNode("mic", micInput);
graph.createNode("gain", gain);
graph.connect("mic", "out", "gain", "in");

AudioEngine audio(&graph);
audio.start(44100, 512, 2, 1); // 2 outputs, 1 input
audio.mapOutputChannel(0, "gain", 0); // Left
audio.mapOutputChannel(1, "gain", 0); // Right
```

## Multiple Input Channels

```cpp
audio.start(44100, 512, 2, 2); // 2 outputs, 2 inputs

auto mic1 = std::make_shared<AudioInputNode>("mic1", 0);
auto mic2 = std::make_shared<AudioInputNode>("mic2", 1);

graph.createNode("mic1", mic1);
graph.createNode("mic2", mic2);

// Process independently
graph.connect("mic1", "out", "effectChain1", "in");
graph.connect("mic2", "out", "effectChain2", "in");
```

## Use Cases

### Live Processing
```cpp
mic → effects → output
```

### Recording/Analysis
```cpp
mic → visualization node
mic → level meter
mic → spectral analyzer
```

### Feedback Warning
When routing microphone input to speakers, **lower your volume** to avoid feedback loops!

## Latency

Total latency = input buffer + processing + output buffer  
At 44100 Hz with 512 samples: ~23ms round-trip latency

## Hardware Support

- **Mono input**: Most laptops (built-in mic)
- **Stereo input**: USB audio interfaces, external sound cards
- **Multi-channel input**: Professional audio interfaces (4-32+ channels)

Use `ma_context_get_devices()` to query available input devices and channel counts.
