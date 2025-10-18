# Node Fade-In System

## Overview

Ogni nodo in MicroSuono può avere un **fade-in automatico** quando viene attivato, prevenendo click e artefatti all'inizio della riproduzione.

Il sistema è **completamente automatico** e **configurabile per nodo**, proprio come il fade-out dell'AudioEngine ma a livello individuale.

---

## Caratteristiche

✅ **Configurabile per nodo** - Ogni nodo può avere una durata diversa  
✅ **Opzionale** - Default 50ms, può essere disabilitato (0ms)  
✅ **Automatico** - Applicato internamente, nessun codice extra nel `process()`  
✅ **Reset-able** - Può essere riattivato con `resetFadeIn()`  
✅ **Click-free** - Eliminazione garantita dei click all'avvio  

---

## API

### Configurazione

```cpp
// Set fade-in duration (in milliseconds)
node->setFadeInDuration(30.0f);  // 30ms fade-in

// Get current fade-in duration
float duration = node->getFadeInDuration();

// Disable fade-in
node->setFadeInDuration(0.0f);  // No fade-in

// Reset fade-in (re-trigger when re-activating)
node->resetFadeIn();
```

### Default

- **Default duration**: 50ms
- **Default state**: Enabled
- **Behavior**: Linear fade from 0 to 1 over the specified duration

---

## Esempi

### Example 1: Quick Fade for Oscillators

```cpp
auto sine = std::make_shared<ms::SineNode>("sine", 440.0f);
sine->setFadeInDuration(30.0f);  // Fast 30ms fade
```

**Rationale**: Oscillatori beneficiano di fade-in brevi per partire velocemente ma senza click.

### Example 2: Smooth Fade for Audio Input

```cpp
auto mic = std::make_shared<ms::AudioInputNode>("mic", 0);
mic->setFadeInDuration(100.0f);  // Smooth 100ms fade
```

**Rationale**: Input audio può beneficiare di fade più lunghi per transizioni graduali.

### Example 3: No Fade for Effects

```cpp
auto gain = std::make_shared<ms::GainNode>("gain", 1.0f);
gain->setFadeInDuration(0.0f);  // No fade-in
```

**Rationale**: Se l'input è già smoothato, il gain potrebbe non aver bisogno di fade-in.

### Example 4: Re-triggering Fade

```cpp
// Stop processing
engine.stop();

// Later, when restarting
sine->resetFadeIn();  // Re-trigger fade-in
engine.start();
```

---

## Implementazione Interna

### Node Base Class

Il fade-in è implementato nel metodo protetto `applyFadeIn()`:

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

### Node Subclass Usage

Ogni nodo chiama `applyFadeIn()` sui suoi output buffer:

```cpp
void SineNode::process(const float* const* audioInputs, 
                       float** audioOutputs, 
                       int nFrames) {
  for (int i = 0; i < nFrames; ++i) {
    audioOutputs[0][i] = amplitude_ * std::sin(phase_);
    phase_ += phaseIncrement_;
    // ... phase wrapping ...
  }
  
  // Apply automatic fade-in
  applyFadeIn(audioOutputs[0], nFrames);
}
```

---

## MicroScript Syntax

In MicroScript, il fade-in si configura con il decoratore `@fadein`:

```microscript
node SineOscillator {
  -> frequency control = 440 @smooth
  <- out audio
  
  @fadein(30ms)  // Configure fade-in duration
  @process {
    out = sin(phase)
  }
}
```

### Opzioni

```microscript
@fadein(0ms)     // Disabled
@fadein(10ms)    // Very fast
@fadein(30ms)    // Fast (good for oscillators)
@fadein(50ms)    // Default
@fadein(100ms)   // Smooth
@fadein(200ms)   // Very smooth
```

**Nota**: Se `@fadein` non è specificato, il default è 50ms.

---

## Differenze con AudioEngine Fade-Out

| Feature | Node Fade-In | AudioEngine Fade-Out |
|---------|--------------|----------------------|
| **Scope** | Per-node | Global (all nodes) |
| **Default** | 50ms | 300ms |
| **When** | On node activation | On engine stop |
| **Configurable** | Yes, per node | Yes, global |
| **Auto-reset** | On `prepare()` | On `stop()` |

---

## Best Practices

### 1. **Oscillators: 20-50ms**
Fast fade-in per partire velocemente senza click.

```cpp
sine->setFadeInDuration(30.0f);
```

### 2. **Audio Input: 50-100ms**
Fade più smooth per microfoni e line-in.

```cpp
mic->setFadeInDuration(80.0f);
```

### 3. **Effects/Processing: 0-30ms**
Effetti possono non aver bisogno di fade se l'input è già smoothato.

```cpp
reverb->setFadeInDuration(0.0f);  // No fade
```

### 4. **LFOs: 0ms**
LFO non hanno bisogno di fade-in, anzi può essere dannoso.

```cpp
lfo->setFadeInDuration(0.0f);
```

### 5. **Tremolo/Modulation: 50-100ms**
Effetti di modulazione beneficiano di fade smooth.

```cpp
tremolo->setFadeInDuration(80.0f);
```

---

## Troubleshooting

### Click all'avvio nonostante fade-in?

**Possibili cause:**
1. Fade-in troppo breve (prova ad aumentare)
2. Fase iniziale non-zero nell'oscillatore
3. DC offset nell'input

**Soluzioni:**
```cpp
// Aumenta durata
node->setFadeInDuration(100.0f);

// Reset fase negli oscillatori (già fatto in prepare())
```

### Fade-in troppo lento?

```cpp
// Riduci durata
node->setFadeInDuration(10.0f);

// O disabilita
node->setFadeInDuration(0.0f);
```

### Fade-in non si resetta?

```cpp
// Chiama resetFadeIn() prima di riattivare
node->resetFadeIn();
```

---

## Implementation Checklist

Per aggiungere fade-in a un nuovo nodo:

- [ ] Chiama `applyFadeIn()` su ogni output buffer alla fine di `process()`
- [ ] Nessun'altra modifica necessaria! Il Node base gestisce tutto

Esempio:

```cpp
void MyNode::process(const float* const* audioInputs,
                     float** audioOutputs,
                     int nFrames) {
  // Processing...
  for (int i = 0; i < nFrames; ++i) {
    audioOutputs[0][i] = /* your DSP */;
  }
  
  // Apply fade-in (that's it!)
  applyFadeIn(audioOutputs[0], nFrames);
}
```

---

## Conclusione

Il sistema di fade-in per nodo è:

✅ **Semplice** - Una linea di configurazione  
✅ **Automatico** - Gestito internamente  
✅ **Flessibile** - Configurabile per nodo  
✅ **Efficace** - Elimina click all'avvio  

Perfetto per creare esperienze audio professionali e click-free! 🎵
