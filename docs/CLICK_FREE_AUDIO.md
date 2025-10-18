# Click-Free Audio System in MicroSuono

## Overview

MicroSuono implementa un sistema completo per eliminare **click** e **artefatti audio** in ogni fase della riproduzione, dal livello di nodo al livello di engine.

---

## Tre Livelli di Protezione

### 1. **Parameter Smoothing** (Control-Rate)
Elimina click quando i parametri cambiano durante la riproduzione.

```cpp
// C++
auto gain = std::make_shared<GainNode>("gain", 1.0f);
// Smoothing automatico per parametri control-rate

// MicroScript
node Gain {
  -> gain control = 1.0 @smooth  // Automatic smoothing!
}
```

**Quando**: Parametri controllati da slider, automation, GUI  
**Durata**: ~11ms (un blocco audio @ 44.1kHz, 512 samples)  
**Algoritmo**: JSFX-style linear interpolation  

---

### 2. **Node Fade-In** (Per-Node)
Elimina click all'avvio di ogni singolo nodo.

```cpp
// C++
auto sine = std::make_shared<SineNode>("sine", 440.0f);
sine->setFadeInDuration(30.0f);  // 30ms fade-in

// MicroScript
node SineOscillator {
  @fadein(30ms)  // Smooth start
  @process {
    out = sin(phase)
  }
}
```

**Quando**: Nodo viene attivato/preparato  
**Durata**: Configurabile (default 50ms)  
**Algoritmo**: Linear fade 0→1  
**Scope**: Individuale per nodo  

---

### 3. **Engine Fade-Out** (Global)
Elimina click quando l'audio engine si ferma.

```cpp
// C++
audio.setFadeOutDuration(300.0f);  // 300ms fade-out
audio.stop();  // Smooth fade to silence

// No MicroScript equivalent (engine-level)
```

**Quando**: `AudioEngine::stop()` viene chiamato  
**Durata**: Configurabile (default 300ms)  
**Algoritmo**: Linear fade 1→0  
**Scope**: Globale (tutti i nodi)  

---

## Confronto Completo

| Feature | Parameter Smoothing | Node Fade-In | Engine Fade-Out |
|---------|---------------------|--------------|-----------------|
| **Trigger** | Cambio parametro | Node activation | Engine stop |
| **Scope** | Per parametro | Per nodo | Globale |
| **Default** | Automatico con `@smooth` | 50ms | 300ms |
| **Configurabile** | Via `@smooth(time)` | Via `setFadeInDuration()` | Via `setFadeOutDuration()` |
| **Disabilitabile** | Rimuovi `@smooth` | `setFadeInDuration(0)` | `setFadeOutDuration(0)` |
| **Algoritmo** | JSFX linear | Linear 0→1 | Linear 1→0 |
| **Quando serve** | GUI controls | Oscillatori, input | Sempre on stop |
| **MicroScript** | `@smooth` | `@fadein(time)` | N/A (engine) |

---

## Esempi Pratici

### Example 1: Oscillatore Click-Free

```cpp
auto sine = std::make_shared<SineNode>("sine", 440.0f);
sine->setFadeInDuration(30.0f);  // Fast fade-in at start

auto gain = std::make_shared<GainNode>("gain", 0.5f);
gain->setFadeInDuration(0.0f);   // No fade (input already fades)

// AudioEngine has fade-out by default
audio.setFadeOutDuration(300.0f);
```

**Result**:
- ✅ No click at start (fade-in 30ms)
- ✅ No click when gain changes (automatic smoothing)
- ✅ No click at stop (fade-out 300ms)

---

### Example 2: Tremolo Effect

```cpp
auto carrier = std::make_shared<SineNode>("carrier", 440.0f, 0.3f);
carrier->setFadeInDuration(50.0f);  // Smooth start for carrier

auto lfo = std::make_shared<SineNode>("lfo", 4.0f, 0.2f, 0.5f);
lfo->setFadeInDuration(0.0f);  // No fade for LFO (modulator)

auto gain = std::make_shared<GainNode>("gain", 1.0f);
gain->setFadeInDuration(0.0f);  // No fade (input fades)

graph.connect("carrier", "out", "gain", "in");
graph.connect("lfo", "out", "gain", "gain");  // Audio-rate modulation
```

**Result**:
- ✅ Carrier fades in smoothly (50ms)
- ✅ LFO starts immediately (no fade needed)
- ✅ Tremolo effect is click-free
- ✅ Audio-rate modulation bypasses parameter smoothing

---

### Example 3: GUI Slider Control

```microscript
node Volume {
  -> in audio
  -> volume control = 0.5 @smooth @min(0) @max(1)
  <- out audio
  
  @fadein(50ms)
  @process {
    out = in * volume
  }
}
```

**Behavior**:
1. Node starts with 50ms fade-in
2. User moves slider: automatic smoothing over ~11ms
3. Engine stops: automatic fade-out over 300ms

**Result**: Zero clicks in tutto il ciclo di vita! 🎉

---

## Best Practices

### Oscillatori
```cpp
oscillator->setFadeInDuration(30.0f);  // Fast fade (20-50ms)
```
**Rationale**: Partono velocemente ma senza click.

### Audio Input (Microfono, Line-In)
```cpp
mic->setFadeInDuration(80.0f);  // Smooth fade (50-100ms)
```
**Rationale**: Transizioni graduali per input esterni.

### Effects (Gain, Filter, ecc.)
```cpp
effect->setFadeInDuration(0.0f);  // No fade
```
**Rationale**: Se l'input è già smoothato, non serve doppio fade.

### LFO / Modulators
```cpp
lfo->setFadeInDuration(0.0f);  // No fade
```
**Rationale**: LFO devono partire immediatamente per modulare correttamente.

### Engine Fade-Out
```cpp
audio.setFadeOutDuration(300.0f);  // 300-500ms
```
**Rationale**: Fade lungo e smooth quando si ferma tutto.

---

## Dual-Mode: Audio-Rate vs Control-Rate

Il sistema gestisce **automaticamente** due modalità per i parametri:

### Control-Rate (con Smoothing)
```cpp
gainNode->setControlValue("gain", 0.8f);  // From slider/automation
// → Smoothing applicato automaticamente
```

### Audio-Rate (senza Smoothing)
```cpp
graph.connect("lfo", "out", "gain", "gain");  // LFO → gain
// → Smoothing bypassato, modulation diretta
```

**Il sistema rileva automaticamente quale modalità usare!**

---

## Algoritmi Implementati

### 1. JSFX-Style Parameter Smoothing
```cpp
if (needsSmoothing_) {
  deltaGain_ = (targetGain_ - currentGain_) / samplesPerBlock_;
  needsSmoothing_ = false;
}

for (int i = 0; i < samplesPerBlock_; i++) {
  output[i] = input[i] * currentGain_;
  currentGain_ += deltaGain_;
}

currentGain_ = targetGain_;  // Snap at end
```

### 2. Linear Fade-In
```cpp
void applyFadeIn(float* buffer, int nFrames) {
  if (!fadeInActive_) return;
  
  for (int i = 0; i < nFrames; ++i) {
    if (currentFadeSample_ < fadeInSamples_) {
      float fadeGain = static_cast<float>(currentFadeSample_) / fadeInSamples_;
      buffer[i] *= fadeGain;
      currentFadeSample_++;
    } else {
      fadeInActive_ = false;
      break;
    }
  }
}
```

### 3. Linear Fade-Out
```cpp
void AudioEngine::stop() {
  if (fadeOutSamples_ == 0) {
    // Immediate stop
    ma_device_uninit(&device_);
    return;
  }
  
  fadeOutActive_ = true;
  currentFadeSample_ = 0;
  
  // Wait for fade to complete
  while (fadeOutActive_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  
  ma_device_uninit(&device_);
}
```

---

## MicroScript Syntax Summary

### Parameter Smoothing
```microscript
-> frequency control = 440 @smooth        // Auto (default block time)
-> cutoff control = 1000 @smooth(20ms)    // Custom duration
-> pan control = 0.5                      // No smoothing
```

### Node Fade-In
```microscript
@fadein(50ms)   // Configure fade-in
@fadein(0ms)    // Disable fade-in
// No @fadein → default 50ms
```

### Complete Example
```microscript
node LowPassFilter {
  -> in audio
  -> cutoff control = 1000 @smooth(15ms) @min(20) @max(20000)
  -> resonance control = 0.5 @smooth(10ms) @min(0) @max(1)
  <- out audio
  
  @fadein(40ms)  // Smooth start
  @process {
    out = lowpass(in, cutoff, resonance)
  }
}
```

---

## Troubleshooting

### Click ancora presenti?

**1. Parametri non smoothati**
```microscript
-> gain control = 1.0 @smooth  // ✅ Add @smooth
```

**2. Fade-in troppo breve**
```cpp
node->setFadeInDuration(100.0f);  // Increase to 100ms
```

**3. Fase iniziale non-zero**
```cpp
// In prepare(), reset phase
phase_ = 0.0f;
```

**4. DC offset**
```cpp
// Use offset parameter
auto sine = std::make_shared<SineNode>("sine", 440, 1.0, 0.0);  // No DC
```

**5. Fade-out troppo breve**
```cpp
audio.setFadeOutDuration(500.0f);  // Longer fade-out
```

---

## Performance

### CPU Impact

| Feature | CPU Cost | Notes |
|---------|----------|-------|
| Parameter Smoothing | ~0.1% | Solo quando parametro cambia |
| Node Fade-In | ~0.05% | Solo nei primi N samples |
| Engine Fade-Out | ~0.05% | Solo durante stop |

**Total overhead**: < 0.3% in scenari tipici

### Memory Impact

| Feature | Memory per Node |
|---------|-----------------|
| Smoothing state | ~16 bytes per parametro |
| Fade-in state | ~12 bytes |
| **Total** | ~28 bytes per node |

**Trascurabile** anche con centinaia di nodi.

---

## Conclusione

Il sistema click-free di MicroSuono è:

✅ **Completo** - Copre start, runtime, stop  
✅ **Automatico** - Minimo codice utente  
✅ **Configurabile** - Tutte le durate personalizzabili  
✅ **Efficiente** - Overhead minimo  
✅ **Dual-Mode** - Audio-rate e control-rate  
✅ **Professionale** - Quality audio production  

**Zero click, zero stress!** 🎵✨
