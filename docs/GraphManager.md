# GraphManager - Data Structures

## Core Members Overview

- `nodes_`: Fast lookup by node ID (unordered_map)
- `orderedNodes_`: Processing order (topological sort)
- `connections_`: List of all connections between nodes
- `audioBuffers_`: Stores audio data for each node's output ports
- `controlValues_`: Stores control data for each node
- `eventBuffers_`: Stores event queues for each node's ports
- `sampleRate_`, `blockSize_`: Audio configuration

## 1. nodes_

**Type**: `unordered_map<string, NodePtr>`

**Purpose**: Fast lookup by node ID. Used when connecting nodes or accessing specific nodes by name.

**Structure**:
```
nodes_ (unordered_map)
┌─────────┬──────────────────────┐
│ "sine"  │ → shared_ptr<SineNode>  │
│ "gain"  │ → shared_ptr<GainNode>  │
│ "output"│ → shared_ptr<OutputNode>│
└─────────┴──────────────────────┘
```

## 2. orderedNodes_

**Type**: `vector<NodePtr>`

**Purpose**: Processing order computed by topological sort. Ensures each node processes AFTER its inputs.

**Why needed**: If gain processes before sine, it reads empty buffers. Topological sort prevents this.

**Structure**:
```
orderedNodes_ (vector)
[0] → SineNode   (no inputs, processes first)
[1] → GainNode   (depends on sine)
[2] → OutputNode (depends on gain)
```

Updated by `sortNodes()` whenever connections change.

## 3. connections_

**Type**: `vector<Connection>`

**Purpose**: Stores all edges in the graph. Each Connection has: `fromNodeId`, `fromPortName`, `toNodeId`, `toPortName`.

**Structure**:
```
connections_ (vector)
[0]: {"sine", "out",  "gain",   "in"}    // sine.out → gain.in
[1]: {"ctrl", "value","sine",   "freq"}  // ctrl.value → sine.freq
[2]: {"gain", "out",  "output", "in"}    // gain.out → output.in
```

**Visual representation**:
```
┌──────┐ out    in ┌──────┐ out    in ┌────────┐
│ sine ├──────────→│ gain ├──────────→│ output │
└──┬───┘           └──────┘           └────────┘
   ↑ freq
   │
┌──┴─────┐ value
│control │
└────────┘
```

Port type is validated when connecting, but not stored in Connection struct.

## 4. audioBuffers_

**Type**: `unordered_map<string, vector<vector<float>>>`

**Purpose**: Stores output audio buffers for each node.

**Structure**:
```
audioBuffers_
┌─────────┬────────────────────────────────────┐
│ "sine"  │ → vector<vector<float>>            │
│         │    [0]: [0.0, 0.5, ..., -0.3]      │
│         │         (512 floats)                │
│ "gain"  │ → vector<vector<float>>            │
│         │    [0]: [0.0, 0.0, ..., 0.0]       │
│         │         (512 floats)                │
│ "output"│ → vector<vector<float>>            │
│         │    [0]: [0.0, 0.0, ..., 0.0]       │
│         │         (512 floats)                │
└─────────┴────────────────────────────────────┘
```

Key = node ID  
Value = vector of buffers (one per audio output port)

**Access**:
- `audioBuffers_["sine"][0]` = first output port buffer of sine
- `audioBuffers_["gain"][0][10]` = 11th sample of first output

## 5. controlValues_

**Type**: `unordered_map<string, unordered_map<string, ControlValue>>`

**Purpose**: Stores control input values for each node (nested: node → port → value).

**Structure**:
```
controlValues_
┌─────────┬────────────────────────────────┐
│ "sine"  │ → unordered_map                │
│         │    ["freq"] = 440.0f (float)   │
│ "gain"  │ → unordered_map                │
│         │    ["gain"] = 0.5f (float)     │
│ "output"│ → {} (empty, no controls)      │
└─────────┴────────────────────────────────┘
```

**Access**:
- `controlValues_["sine"]["freq"]` = get frequency
- `controlValues_["gain"]["gain"] = 0.8f` = set gain

## 6. eventBuffers_

**Type**: `unordered_map<string, unordered_map<string, vector<Event>>>`

**Purpose**: Stores event queues for each node's event ports (nested: node → port → events).

**Structure**:
```
eventBuffers_
┌──────────┬────────────────────────────────┐
│ "trigger"│ → unordered_map                │
│          │    ["out"] = [Event{"bang", 0.0, 128}]│
│ "sequenc"│ → unordered_map                │
│          │    ["notes"] = [Event{"noteOn", 60, 0},│
│          │                 Event{"noteOff", 60, 256}]│
└──────────┴────────────────────────────────┘
```

Each event has `sampleOffset` for sample-accurate timing within the block.  
Cleared at the start of each `process()` call.

**Access**:
- `eventBuffers_["trigger"]["out"]` = vector of events for that port
- Events can be generated at any time during node processing

## 7. sampleRate_ and blockSize_

Audio configuration set by `prepare()`.  
Used to allocate buffers and passed to nodes for internal calculations (e.g., oscillator phase increment).

## Processing Flow

```
process(nFrames) called
       ↓
┌────────────────────────────────────────┐
│ 1. Clear eventBuffers_                 │
│    Empty all event queues              │
└────────────────────────────────────────┘
       ↓
┌────────────────────────────────────────┐
│ 2. Process controls in orderedNodes_   │
│    For each node:                      │
│      node->processControl(             │
│        controlValues_[node->id], ...)  │
│                                        │
│    Example:                            │
│      sine reads controlValues_["sine"]["freq"]│
│      → Updates internal frequency_     │
└────────────────────────────────────────┘
       ↓
┌────────────────────────────────────────┐
│ 3. Process events in orderedNodes_     │
│    For each node:                      │
│      node->processEvents(              │
│        eventInputs, eventOutputs)      │
│                                        │
│    Example:                            │
│      threshold: Check if signal > 0.5  │
│      → Generate Event{"trigger", 1.0, sampleOffset}│
│      → Store in eventBuffers_["threshold"]["out"]│
└────────────────────────────────────────┘
       ↓
┌────────────────────────────────────────┐
│ 4. Process audio in orderedNodes_      │
│    For each node:                      │
│      Collect input buffer pointers     │
│      Collect output buffer pointers    │
│      node->process(inputs, outputs, nFrames)│
│                                        │
│    Example:                            │
│      sine: Generate → audioBuffers_["sine"][0]│
│      gain: Read sine's buffer, multiply│
│            → audioBuffers_["gain"][0]  │
└────────────────────────────────────────┘
       ↓
Done (AudioEngine retrieves output)
```

## Connection Mechanism

`connect("sine", "out", "gain", "in")`:

1. Validates port types match
2. Creates Connection and adds to `connections_`
3. Calls `sortNodes()` to update `orderedNodes_`

**Zero-copy data flow during `process()`**:
```
sine writes to:  audioBuffers_["sine"][0]
                        ↓
                  (pointer passed)
                        ↓
gain reads from: &audioBuffers_["sine"][0]  ← same memory!
gain writes to:  audioBuffers_["gain"][0]
                        ↓
                  (pointer passed)
                        ↓
output reads:    &audioBuffers_["gain"][0]  ← same memory!
```

GraphManager builds pointer arrays for each node's `process()` call.  
No data copying - nodes read/write shared buffers directly.
