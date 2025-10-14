# AudioEngine - Data Structures

## Core Members

- `graph_`: Pointer to external GraphManager (not owned by AudioEngine)
- `device_`: miniaudio device handle (struct, not pointer)
- `blockSize_`: Number of samples per callback

## Audio Callback Flow

1. OS/Audio device requests samples → calls `audioCallback()` (real-time thread)
2. `audioCallback()` calls `graph_->process(nFrames)`
3. GraphManager processes all nodes, fills buffers
4. `audioCallback()` retrieves output via `getNodeOutput()`
5. Copies to miniaudio output buffer → sent to speakers

## Buffer Layout

miniaudio uses interleaved stereo: `[L0, R0, L1, R1, ...]`

Current implementation duplicates mono graph output to both channels:
```
Graph: [0.5, 0.3, ...]
    ↓
Output: [0.5, 0.5, 0.3, 0.3, ...]  (L, R, L, R, ...)
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
- Channels: 2 (stereo)
- Sample rate: 48000 Hz (configurable in `start()`)
- Callback: `audioCallback` static function
- User data: `this` pointer passed to callback

## Lifecycle

1. Constructor: Initialize miniaudio device
2. `start(sampleRate, blockSize)`: Spawn audio thread, begin playback
3. (Audio thread calls callback continuously)
4. `stop()`: Stop audio thread, cleanup
