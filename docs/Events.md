# Event System - Usage Guide

## Overview
Events are discrete messages that can be generated at any sample time during processing.  
Unlike audio (continuous) and control (per-block), events have **sample-accurate timing**.

## Event Structure
```
Event
├── type: string (e.g., "trigger", "noteOn", "bang")
├── value: ControlValue (optional data)
└── sampleOffset: int (0 to blockSize-1)
```

## When to Use Events
- **Triggers**: Generate bang/pulse when condition is met
- **Note messages**: MIDI-style note on/off
- **State changes**: Discrete notifications (play/stop, mode switch)
- **Sequencing**: Timed events for pattern generation

## Example: Threshold Node (generates trigger event)

**Concept**: Monitor audio input, generate "trigger" event when signal crosses threshold.

**Ports**:
- Input: `{"in", Audio}`, `{"threshold", Control}`
- Output: `{"trigger", Event}`

**Processing flow**:
```
processEvents():
  Check if audio crossed threshold during last block
  If yes: Generate Event{"trigger", 1.0, sampleOffset}
  Write to eventOutputs["trigger"]
```

**Usage in graph**:
```
sine → threshold → envelope
       (trigger)
```

When sine crosses threshold, trigger event fires → envelope node receives it → starts attack phase.

## Example: Event Generator Node

**Concept**: Generate periodic events (like a metronome).

**Ports**:
- Input: `{"bpm", Control}`
- Output: `{"tick", Event}`

**Processing**:
```
processEvents():
  Calculate samples per beat from bpm and sampleRate
  If accumulated samples >= samplesPerBeat:
    Generate Event{"tick", beat_number, sampleOffset}
    Reset counter
```

## Sample-Accurate Timing

The `sampleOffset` field enables **sample-accurate event timing**:
- Range: 0 to (blockSize - 1)
- Example: Event at `sampleOffset = 128` occurs 128 samples into the block

Receiving node can:
- Process event immediately (simple case)
- Schedule action at exact sample time (advanced)

## Implementation Pattern

**Generating events in a node**:
```cpp
void MyNode::processEvents(
  const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
  std::unordered_map<std::string, std::vector<Event>>& eventOutputs) 
{
  // Check condition (e.g., threshold crossed)
  if (conditionMet) {
    // Create event with sample-accurate timing
    Event evt("trigger", 1.0f, sampleOffset);
    
    // Add to output queue
    eventOutputs["out"].push_back(evt);
  }
}
```

**Receiving events in a node**:
```cpp
void MyNode::processEvents(
  const std::unordered_map<std::string, std::vector<Event>>& eventInputs,
  std::unordered_map<std::string, std::vector<Event>>& eventOutputs) 
{
  auto it = eventInputs.find("trigger_in");
  if (it != eventInputs.end()) {
    for (const Event& evt : it->second) {
      if (evt.type == "trigger") {
        // React to trigger
        startEnvelope();
      }
    }
  }
}
```

## Event Flow in GraphManager

```
Block N processing:
  1. Clear all eventBuffers_
  2. processControl() for all nodes
  3. processEvents() for all nodes (in orderedNodes_ order)
     - Nodes read input events
     - Nodes generate output events
     - Events stored in eventBuffers_[nodeId][portName]
  4. process() for all nodes
  
Block N+1 processing:
  - Events from block N are propagated via connections
  - New events can be generated
```

## Key Points

- **Non-periodic**: Events occur when conditions are met, not at fixed intervals
- **Sample-accurate**: Each event has precise timing via `sampleOffset`
- **Zero-copy**: Events passed by reference through connections
- **Flexible data**: `value` field can hold float/int/bool/string via ControlValue variant
- **Multiple events**: A port can have multiple events in one block (vector)
