# Node System - Data Structures

## Node Base Class Members

**Public:**
- `id`: Unique identifier, const (set once, never changes)
- `params`: Vector of Param (name/value pairs) for UI/scripting (not currently used)

**Protected:**
- `inputPorts_` / `outputPorts_`: Vectors of Port descriptors
- `sampleRate_` / `blockSize_`: Set by `prepare()`, used for calculations

## Param Struct
Simple name-value pair for exposing parameters to external systems.  
Currently available but not extensively used - reserved for future extensions.

## Example: SineNode

**Structure:**
```
SineNode "sine"
├── id = "sine" (const, public)
├── params = [] (empty)
├── inputPorts_:  [Port{"freq", Control}]
├── outputPorts_: [Port{"out", Audio}]
├── sampleRate_ = 48000
├── blockSize_ = 512
└── Private state:
    ├── frequency_ = 440.0f
    ├── phase_ = 0.0f
    └── phaseIncrement_ (calculated)
```

**Lifecycle:**
1. Constructor: Sets id, initial frequency, adds ports
2. `prepare()`: Calculates `phaseIncrement_` from sample rate
3. `processControl()`: Reads "freq" input, updates frequency and recalculates increment
4. `process()`: Generates sine samples, advances phase, writes to output buffer

## Example: GainNode

**Structure:**
```
GainNode "gain"
├── id = "gain" (const, public)
├── params = [] (empty)
├── inputPorts_:  [Port{"in", Audio}, Port{"gain", Control}]
├── outputPorts_: [Port{"out", Audio}]
├── sampleRate_ = 48000
├── blockSize_ = 512
└── Private state:
    └── gain_ = 1.0f
```

**Processing:**
1. `processControl()`: Reads "gain" control input, updates `gain_` member
2. `process()`: Multiplies each sample from input[0] by `gain_`, writes to output[0]

## Port Registration

In constructor, call `addInputPort()` / `addOutputPort()` helper methods.  
Order matters: index 0, 1, 2... matches array indices in `process()` buffer arguments.

## Process Methods

**processControl()**
- Called once per block, before events and audio
- Receives map of control input values by port name
- Updates internal node state (e.g., frequency, gain)
- Can write to control output map

**processEvents()**
- Called once per block, after controls, before audio
- Receives map of event queues by port name (each queue = vector of Events)
- Can generate output events at any time during the block
- Each event has `sampleOffset` for sample-accurate timing
- Example: Generate "trigger" event when condition is met

**process()**
- Called once per block for audio processing
- Receives arrays of buffer pointers (one per audio port)
- `audioInputs[i]` = pointer to i-th input port buffer
- `audioOutputs[i]` = pointer to i-th output port buffer
- Must process exactly `nFrames` samples
- Can generate events during processing (store them for next block)
