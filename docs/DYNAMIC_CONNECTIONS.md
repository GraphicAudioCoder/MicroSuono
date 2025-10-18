# Dynamic Connection System (Live Patching)

MicroSuono supports **runtime modification** of the audio graph, similar to Pure Data, Max/MSP, and other visual programming environments. You can add/remove nodes and change connections **while audio is playing** without crashes or buffer underruns.

## 🎯 Features

### Thread-Safe Operations
All graph modifications are protected by mutex locks to prevent race conditions between the audio thread and the UI/control thread.

```cpp
// Safe to call from any thread while audio is playing
graph.connect("sine", "out", "gain", "in");
graph.disconnect("sine", "out", "gain", "in");
graph.createNode("newNode", nodePtr);
graph.removeNode("oldNode");
```

### Hot-Pluggable Nodes
New nodes are automatically prepared when added to a running graph:

```cpp
// Graph is already running
auto newSine = std::make_shared<SineNode>("sine2", 440.0f);
graph.createNode("sine2", newSine);  // ✓ Automatically prepared!
graph.connect("sine2", "out", "mixer", "in_1");
```

### Graceful Degradation
If the audio thread tries to process while the graph is being modified, it outputs silence for that block instead of crashing or blocking:

```cpp
void GraphManager::process(int nFrames) {
  // Try to acquire lock without blocking audio thread
  std::unique_lock<std::mutex> lock(graphMutex_, std::try_to_lock);
  if (!lock.owns_lock()) {
    // Graph is being modified - output silence this block
    return;
  }
  
  // Safe to process...
}
```

## 📚 API Reference

### Connection Management

#### `connect()`
Connect two nodes by port name. Thread-safe.

```cpp
graph.connect("source", "outputPort", "destination", "inputPort");
```

**Example:**
```cpp
graph.connect("sine", "out", "gain", "in");
graph.connect("lfo", "out", "filter", "cutoff");
```

#### `disconnect()`
Disconnect a specific connection. Thread-safe.

```cpp
bool success = graph.disconnect("source", "out", "dest", "in");
```

**Returns:** `true` if connection existed and was removed, `false` otherwise.

### Node Management

#### `createNode()`
Add a node to the graph. If graph is prepared, node is hot-plugged.

```cpp
auto node = std::make_shared<SomeNode>("id", params...);
graph.createNode("id", node);
```

**Example:**
```cpp
// Add reverb effect while audio is playing
auto reverb = std::make_shared<ReverbNode>("reverb");
graph.createNode("reverb", reverb);
graph.connect("gain", "out", "reverb", "in");
```

#### `removeNode()`
Remove a node and all its connections. Thread-safe.

```cpp
graph.removeNode("nodeId");
```

**Automatically:**
- Disconnects all incoming and outgoing connections
- Frees allocated buffers
- Removes from processing order

### Query Operations

#### `getNode()`
Get node pointer by ID (const-safe).

```cpp
auto node = graph.getNode("sine");
if (node) {
  // Cast to specific type
  auto sine = std::dynamic_pointer_cast<SineNode>(node);
  sine->setFrequency(880.0f);
}
```

## 🎼 Usage Patterns

### Pattern 1: Effect Bypass
Toggle effect on/off by reconnecting:

```cpp
bool effectEnabled = true;

void toggleEffect() {
  if (effectEnabled) {
    // Bypass: connect directly
    graph.disconnect("source", "out", "effect", "in");
    graph.disconnect("effect", "out", "output", "in");
    graph.connect("source", "out", "output", "in");
  } else {
    // Enable: route through effect
    graph.disconnect("source", "out", "output", "in");
    graph.connect("source", "out", "effect", "in");
    graph.connect("effect", "out", "output", "in");
  }
  effectEnabled = !effectEnabled;
}
```

### Pattern 2: Hot-Swap Effects
Replace one effect with another:

```cpp
void swapReverb() {
  // Remove old reverb
  graph.removeNode("reverb");
  
  // Add new reverb with different settings
  auto newReverb = std::make_shared<ReverbNode>("reverb");
  newReverb->setRoomSize(0.8f);
  graph.createNode("reverb", newReverb);
  
  // Reconnect
  graph.connect("gain", "out", "reverb", "in");
}
```

### Pattern 3: Dynamic Mixing
Add/remove voices in a mixer:

```cpp
int voiceCount = 0;

void addVoice(float frequency) {
  std::string id = "voice" + std::to_string(++voiceCount);
  
  auto sine = std::make_shared<SineNode>(id, frequency);
  graph.createNode(id, sine);
  
  // Connect to mixer
  std::string mixerInput = "in_" + std::to_string(voiceCount - 1);
  graph.connect(id, "out", "mixer", mixerInput);
}

void removeVoice(int voiceNum) {
  std::string id = "voice" + std::to_string(voiceNum);
  graph.removeNode(id);  // Auto-disconnects
}
```

### Pattern 4: Parameter Modulation Routing
Dynamically route modulators to different targets:

```cpp
void routeLFO(const std::string& targetParam) {
  // Disconnect from old target
  graph.disconnect("lfo", "out", "filter", "cutoff");
  graph.disconnect("lfo", "out", "gain", "amount");
  
  // Connect to new target
  graph.connect("lfo", "out", targetNode, targetParam);
}
```

## ⚠️ Limitations & Considerations

### 1. Brief Silence During Modification
When the graph lock is held by a modification operation, the audio thread skips one block (typically ~10-12ms at 44.1kHz/512 samples). This is preferable to audio thread blocking.

**Acceptable:** Rare modifications (user interaction)  
**Not recommended:** Continuous modifications every block

### 2. No Cycle Detection (Yet)
Creating feedback loops can cause crashes:

```cpp
// ⚠️ DANGEROUS: Creates cycle
graph.connect("delay", "out", "filter", "in");
graph.connect("filter", "out", "delay", "in");  // Cycle!
```

**TODO:** Add cycle detection before allowing connection.

### 3. Processing Order
Nodes are processed in creation order, not topological sort. For now, create nodes in dependency order:

```cpp
// ✓ Good: Create in signal flow order
graph.createNode("sine", sine);
graph.createNode("filter", filter);
graph.createNode("reverb", reverb);

// ⚠️ May cause 1-block latency
graph.createNode("reverb", reverb);
graph.createNode("sine", sine);    // Processes after reverb!
graph.createNode("filter", filter);
```

**TODO:** Implement automatic topological sorting.

### 4. Control Rate vs Audio Rate
Connections are processed every audio block. For very fast control rate (sample-accurate), consider internal smoothing in nodes instead of disconnecting/reconnecting.

## 🧪 Testing

Run the included demos to test dynamic behavior:

```bash
# Automated tests
./build/demo9_dynamic_connections

# Interactive live patching
./build/demo10_live_patching
```

### Demo 9: Automated Tests
- ✓ Connect/disconnect while playing
- ✓ Rapid connection changes (no crashes)
- ✓ Hot-add nodes
- ✓ Remove nodes with active connections

### Demo 10: Interactive
Menu-driven interface to:
1. Add/remove effects in real-time
2. Change routing while hearing results
3. Add multiple oscillators dynamically
4. Modify parameters on-the-fly

## 🔬 Implementation Details

### Mutex Strategy
- **`std::mutex graphMutex_`**: Protects all graph structure modifications
- **`std::try_to_lock`** in `process()`: Non-blocking attempt to acquire lock
- **Lock scope**: Held only during structure changes, not entire processing

### Buffer Management
- Buffers allocated per-node on creation
- `allocateBuffersForNode()` called for hot-plugged nodes
- Automatic cleanup on node removal

### Atomic Flags
- **`std::atomic<bool> isPrepared_`**: Tracks if graph is ready for processing
- **Lock-free reads** in critical paths

## 📖 Related Documentation

- **Node.md**: Node lifecycle and preparation
- **GraphManager.md**: Graph topology and processing
- **Port.md**: Connection types and data flow

## 🚀 Future Enhancements

- [ ] Cycle detection before allowing connections
- [ ] Topological sorting for optimal processing order
- [ ] Connection validation (type checking)
- [ ] Undo/redo for graph modifications
- [ ] Serialization/deserialization of graph state
- [ ] Visual graph editor integration

## 💡 Design Philosophy

The dynamic connection system prioritizes:

1. **Stability**: Never crash the audio thread
2. **Simplicity**: Easy API for common operations
3. **Performance**: Minimal overhead in audio processing
4. **Flexibility**: Support creative workflows like Pure Data

**Trade-off:** Brief silence during modifications is acceptable for better stability and simpler code.
