# AudioEngine - Data Structures

## Core Members

- `graph_`: Pointer to external GraphManager (not owned by AudioEngine)
- `device_`: miniaudio device handle (struct, not pointer)
- `blockSize_`: Number of samples per callback
- `numChannels_`: Number of output channels (configurable, default: 2)
- `channelMappings_`: Maps physical channels to graph node outputs

## Audio Callback Flow

### Output-Only Mode
1. OS/Audio device requests samples → calls `audioCallback()` (real-time thread)
2. `audioCallback()` calls `graph_->process(nFrames)`
3. GraphManager processes all nodes, fills buffers
4. `audioCallback()` reads mapped node outputs and interleaves them
5. Copies to miniaudio output buffer → sent to speakers

### Duplex Mode (Input + Output)
1. OS/Audio device provides input samples and requests output → calls `audioCallback()`
2. `audioCallback()` deinterleaves input buffer and injects into GraphManager via `setPhysicalInput()`
3. `audioCallback()` calls `graph_->process(nFrames)`
4. Nodes can access physical inputs via `getPhysicalInput(channelIndex)`
5. GraphManager processes all nodes, fills buffers
6. `audioCallback()` reads mapped node outputs and interleaves them
7. Copies to miniaudio output buffer → sent to speakers

## Multichannel I/O

The engine supports:
- **N output channels** (limited only by hardware)
- **M input channels** (microphone, line-in, etc.)

Configured via `start(sampleRate, blockSize, numOutputChannels, numInputChannels)`.

## Multichannel Output

Output channels map graph nodes to physical outputs.

### Channel Mapping

Use `mapOutputChannel()` to route graph nodes to physical outputs:

```cpp
audio.mapOutputChannel(0, "nodeLeft", 0);  // Ch 0 (L) ← nodeLeft output 0
audio.mapOutputChannel(1, "nodeRight", 0); // Ch 1 (R) ← nodeRight output 0
audio.mapOutputChannel(2, "nodeSub", 0);   // Ch 2 (Sub) ← nodeSub output 0
```

### Buffer Layout

miniaudio uses interleaved format: `[Ch0_0, Ch1_0, ..., ChN_0, Ch0_1, Ch1_1, ...]`

Example with 2 channels (stereo):
```
Graph nodes:         Node A: [0.5, 0.3, ...]
                     Node B: [0.8, 0.1, ...]
                         ↓
Channel mapping:     Ch0 ← Node A
                     Ch1 ← Node B
                         ↓
Interleaved output:  [0.5, 0.8, 0.3, 0.1, ...]
                      L    R    L    R
```

## Real-time Thread Constraints

Callback runs on high-priority thread with strict timing (~11ms @ 512 samples, 48kHz).

**Never:**
- Allocate memory (new, malloc, vector.push_back)
- Lock mutexes (priority inversion risk)
- Do I/O (file, network, console)

**Safe:**
- Read/write pre-allocated buffers
- Math operations
- Simple conditionals

## Device Configuration

- Format: 32-bit float
- Channels: Configurable (default: 2 for stereo, supports up to hardware limit)
- Sample rate: Configurable in `start()` (default: 44100 Hz)
- Callback: `audioCallback` static function
- User data: `this` pointer passed to callback

## Lifecycle

1. Constructor: Initialize miniaudio device
2. `start(sampleRate, blockSize, numChannels)`: Configure device, spawn audio thread, begin playback
3. `mapOutputChannel(...)`: Route graph nodes to physical channels
4. (Audio thread calls callback continuously)
5. `stop()`: Stop audio thread, cleanup

## Audio Input

AudioEngine automatically deinterleaves physical input samples and injects them into GraphManager's physical input buffers.

**No registration required!** Any node can access physical inputs via the protected `getPhysicalInput(channelIndex)` method inherited from Node.

### How It Works

```
Physical Input → AudioEngine deinterleaves → GraphManager.setPhysicalInput()
                                                    ↓
                  Any Node: getPhysicalInput(ch) → Access raw input samples
```

Nodes that want to access physical inputs (like AudioInputNode, or custom DSL nodes) simply call:

```cpp
const float* inputBuffer = getPhysicalInput(channelIndex);
```

This design ensures that **custom DSL nodes** can access audio hardware directly without modifying the C++ engine code.

## Example: Stereo Output

```cpp
GraphManager graph;
auto sineL = std::make_shared<SineNode>("left", 440.0f);
auto sineR = std::make_shared<SineNode>("right", 880.0f);
graph.createNode("left", sineL);
graph.createNode("right", sineR);

AudioEngine audio(&graph);
audio.start(44100, 512, 2);  // 2 output channels
audio.mapOutputChannel(0, "left", 0);   // L ← 440 Hz
audio.mapOutputChannel(1, "right", 0);  // R ← 880 Hz
```

## Example: Audio Input (Duplex)

```cpp
GraphManager graph;
auto micInput = std::make_shared<AudioInputNode>("mic", 0);
auto gain = std::make_shared<GainNode>("gain", 0.5f);
graph.createNode("mic", micInput);
graph.createNode("gain", gain);
graph.connect("mic", "out", "gain", "in");

AudioEngine audio(&graph);
audio.start(44100, 512, 2, 1);  // 2 outputs, 1 input
audio.mapOutputChannel(0, "gain", 0);  // Monitor through left
audio.mapOutputChannel(1, "gain", 0);  // Monitor through right
```
