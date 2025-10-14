# MicroSuono Architecture - Data Flow

## System Layers

```
User Code (main.cpp)
    ↓
GraphManager (nodes, connections, buffers)
    ↓
Audio Callback (real-time thread)
    ↓
Speakers
```

## Data Flow: Sine → Gain → Output Example

### Graph Structure
```
┌──────┐ out    in ┌──────┐ out    in ┌────────┐
│ sine ├──────────→│ gain ├──────────→│ output │
└──────┘           └──────┘           └────────┘
```

### Setup Data Structures
```
nodes_:
┌─────────┬─────────────┐
│ "sine"  │ → SineNode  │
│ "gain"  │ → GainNode  │
│ "output"│ → OutputNode│
└─────────┴─────────────┘

orderedNodes_:
[0] → sine (no deps)
[1] → gain (depends on sine)
[2] → output (depends on gain)

connections_:
[0]: sine.out  → gain.in
[1]: gain.out  → output.in

audioBuffers_:
"sine"   → [[512 floats]]
"gain"   → [[512 floats]]
"output" → [[512 floats]]

controlValues_:
"sine"   → {"freq": 440.0f}
"gain"   → {"gain": 1.0f}
"output" → {}
```

### Process Cycle

Every ~11ms @ 48kHz:

```
process(512) called
       ↓
┌─────────────────────────────────┐
│ 1. Clear audioBuffers_          │
│    Zero all output buffers      │
└─────────────────────────────────┘
       ↓
┌─────────────────────────────────┐
│ 2. Process Controls             │
│    sine.processControl()        │
│      Read freq from controls    │
│    gain.processControl()        │
│      Read gain from controls    │
└─────────────────────────────────┘
       ↓
┌─────────────────────────────────┐
│ 3. Process Audio (in order)     │
│                                 │
│    sine.process()               │
│      Generate 512 samples       │
│      → audioBuffers_["sine"][0] │
│                                 │
│    gain.process()               │
│      Read sine's buffer ────┐   │
│      Multiply by gain       │   │
│      → audioBuffers_["gain"][0] │
│                             │   │
│    output.process()         │   │
│      Read gain's buffer ────┘   │
│      → audioBuffers_["output"][0]│
└─────────────────────────────────┘
       ↓
AudioEngine retrieves output → Speakers
```

## Buffer Timeline

One buffer = 512 samples = ~10.67ms @ 48kHz

```
Index:  0     1     2    ...  510   511
Time:   0ms   0.02  0.04 ...  10.6  10.67ms
```

## Key: Zero-Copy via Pointers

Connections pass buffer pointers, not data:

```
sine writes:  audioBuffers_["sine"][0]
              ↓ (pointer)
gain reads:   same buffer (zero-copy!)
gain writes:  audioBuffers_["gain"][0]
              ↓ (pointer)
output reads: same buffer (zero-copy!)
```

GraphManager builds pointer arrays for `process()` calls.  
Nodes read/write shared memory directly - no data copying.
