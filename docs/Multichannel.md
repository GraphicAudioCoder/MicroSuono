# Multichannel Audio in MicroSuono

MicroSuono supports N-channel audio output, limited only by your audio hardware.

## Architecture

### No Automatic Mono Duplication

Unlike the previous implementation, **mono signals are not automatically duplicated** to stereo. Each physical output channel must be explicitly mapped to a graph node output.

### Channel Mapping

Use `AudioEngine::mapOutputChannel()` to route graph outputs to physical channels:

```cpp
AudioEngine audio(&graph);
audio.start(44100, 512, 2);  // 2 channels = stereo

// Map physical outputs
audio.mapOutputChannel(0, "nodeId", 0);  // Physical channel 0 (Left)
audio.mapOutputChannel(1, "nodeId", 1);  // Physical channel 1 (Right)
```

## Common Scenarios

### 1. Independent Stereo Channels

Two separate mono signals routed to left and right:

```cpp
auto sineL = std::make_shared<SineNode>("left", 440.0f);
auto sineR = std::make_shared<SineNode>("right", 880.0f);

graph.createNode("left", sineL);
graph.createNode("right", sineR);

audio.start(44100, 512, 2);
audio.mapOutputChannel(0, "left", 0);
audio.mapOutputChannel(1, "right", 0);
```

Result: 440 Hz in left ear, 880 Hz in right ear.

### 2. Mono to Stereo with Panning

Use `MonoToStereoNode` to convert mono → stereo with controllable pan:

```cpp
auto sine = std::make_shared<SineNode>("sine", 440.0f);
auto stereo = std::make_shared<MonoToStereoNode>("stereo", 0.0f); // Center

graph.createNode("sine", sine);
graph.createNode("stereo", stereo);
graph.connect("sine", "out", "stereo", "in");

audio.start(44100, 512, 2);
audio.mapOutputChannel(0, "stereo", 0);  // Left output
audio.mapOutputChannel(1, "stereo", 1);  // Right output
```

Result: 440 Hz in both ears (centered), using constant-power panning.

### 3. Multichannel (Surround)

```cpp
audio.start(44100, 512, 6);  // 5.1 surround

audio.mapOutputChannel(0, "frontLeft", 0);
audio.mapOutputChannel(1, "frontRight", 0);
audio.mapOutputChannel(2, "center", 0);
audio.mapOutputChannel(3, "lfe", 0);
audio.mapOutputChannel(4, "rearLeft", 0);
audio.mapOutputChannel(5, "rearRight", 0);
```

### 4. Parallel Processing with Mixing

```cpp
// Two oscillators → two processors → mixer → stereo
auto osc1 = std::make_shared<SineNode>("osc1", 440.0f);
auto osc2 = std::make_shared<SineNode>("osc2", 880.0f);
auto fx1 = std::make_shared<GainNode>("fx1", 0.5f);
auto fx2 = std::make_shared<GainNode>("fx2", 0.3f);
auto mixer = std::make_shared<MixerNode>("mixer");  // (needs implementation)

graph.connect("osc1", "out", "fx1", "in");
graph.connect("osc2", "out", "fx2", "in");
graph.connect("fx1", "out", "mixer", "in1");
graph.connect("fx2", "out", "mixer", "in2");

// Mixer output → mono
audio.mapOutputChannel(0, "mixer", 0);
audio.mapOutputChannel(1, "mixer", 0);  // Duplicate to stereo
```

## Hardware Capabilities

### Query Available Channels

miniaudio can query hardware capabilities:

```cpp
ma_device_info* pPlaybackInfos;
ma_uint32 playbackCount;
ma_context context;

ma_context_init(NULL, 0, NULL, &context);
ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, NULL, NULL);

for (ma_uint32 i = 0; i < playbackCount; ++i) {
    std::cout << "Device: " << pPlaybackInfos[i].name << std::endl;
    std::cout << "Max channels: " << pPlaybackInfos[i].maxChannels << std::endl;
}
```

### Typical Hardware Limits

- **Consumer audio interfaces**: 2 channels (stereo)
- **USB audio interfaces**: 2-8 channels
- **Professional audio interfaces**: 8-32+ channels
- **miniaudio theoretical limit**: 254 channels

## MonoToStereoNode

Dedicated node for mono → stereo conversion with panning.

### Ports

- **Input**: `in` (Audio), `pan` (Control, -1.0 to +1.0)
- **Output**: `left` (Audio), `right` (Audio)

### Panning Law

Uses constant-power panning:
- `-1.0`: Full left (L=1.0, R=0.0)
- `0.0`: Center (L=0.707, R=0.707) ≈ -3dB
- `+1.0`: Full right (L=0.0, R=1.0)

### Dynamic Panning

```cpp
auto lfo = std::make_shared<SineNode>("lfo", 0.5f);  // 0.5 Hz
auto stereo = std::make_shared<MonoToStereoNode>("stereo");

graph.connect("lfo", "out", "stereo", "pan");  // Modulate pan position
```

## Best Practices

1. **Explicit routing**: Always map all channels you intend to use
2. **Unmapped channels are silent**: If you only map channel 0, channel 1 outputs silence
3. **Node outputs are mono**: Each node output port is mono; use MonoToStereoNode or multiple nodes for stereo
4. **Parallel processing**: Multiple nodes can run in parallel and be mixed downstream
5. **Buffer interleaving**: AudioEngine handles interleaving automatically

## See Also

- `docs/AudioEngine.md` - AudioEngine internals
- `docs/MonoToStereoNode.md` - Panning details
- `examples/multichannel_demo.cpp` - Working examples
