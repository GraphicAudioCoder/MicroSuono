# Port System - Data Structures

## Port struct
Simple descriptor with name and type. Stored in each Node's `inputPorts_` and `outputPorts_` vectors.

## PortType enum
Three types:
- **Audio**: Buffer of floats (e.g., 512 samples per block)
- **Control**: Single variant value updated once per block
- **Event**: Discrete message/trigger (non-periodic)

## ControlValue
`std::variant` that can hold: `float`, `int`, `bool`, or `std::string`.  
Used for control data like frequency (440.0f), switches (true/false), or labels ("A4").

---

## Node Port Storage
Nodes store ports in `inputPorts_` and `outputPorts_` vectors.  
Index in vector = port index used in `process()` buffer arrays.

**Example: GainNode structure**
```
Node
├── inputPorts_: vector<Port>
│   ├── [0]: Port{"in", Audio}
│   └── [1]: Port{"gain", Control}
├── outputPorts_: vector<Port>
│   └── [0]: Port{"out", Audio}
```

- Input 0 = audio input buffer
- Output 0 = audio output buffer

Access: Use `getInputPorts()` / `getOutputPorts()` public methods.  
Registration: Call `addInputPort()` / `addOutputPort()` in constructor.

---

## Update Timing
- **Audio**: Every sample in the block (512 values per ~11.6ms @ 44.1kHz)
- **Control**: Once per block (~11.6ms @ 44.1kHz)
- **Event**: Immediate when triggered (non-periodic)
