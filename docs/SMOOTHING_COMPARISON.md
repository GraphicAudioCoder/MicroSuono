# Smoothing: Prima vs Dopo

## Il Problema

Nella prima versione, l'utente doveva gestire manualmente tutto lo smoothing:

### ❌ PRIMA: Complicato e Tedioso

```microscript
// 54 righe di codice per un semplice gain!
node Gain {
  -> in audio
  -> gain control = 1.0 @smooth(auto)
  <- out audio

  // L'utente deve dichiarare tutte queste variabili
  targetGain = gain
  currentGain = gain
  deltaGain = 0
  needsSmoothing = false

  @prepare {
    // L'utente deve inizializzare tutto
    currentGain = gain
    targetGain = gain
    deltaGain = 0
  }

  @control {
    // L'utente deve gestire i cambiamenti
    if (gain != targetGain) {
      targetGain = gain
      needsSmoothing = true
    }
  }

  @process {
    // L'utente deve gestire due modalità
    if (gain.isAudioRate) {
      out = in * gain
    } else {
      // L'utente deve implementare lo smoothing
      if (needsSmoothing) {
        deltaGain = (targetGain - currentGain) / samplesPerBlock
        needsSmoothing = false
      }
      
      out = in * currentGain
      currentGain += deltaGain
      
      // L'utente deve gestire lo snap
      if (sampleIndex == samplesPerBlock - 1) {
        currentGain = targetGain
        deltaGain = 0
      }
    }
  }
}
```

**Problemi:**
- 🔴 Troppo verboso (54 righe!)
- 🔴 L'utente deve capire JSFX smoothing
- 🔴 Errori facili (dimenticare `needsSmoothing`, snap finale, etc.)
- 🔴 Codice ripetuto per ogni parametro smoothato
- 🔴 Focus sulla meccanica invece che sulla logica DSP

---

## La Soluzione

### ✅ DOPO: Semplice e Automatico

```microscript
// 11 righe di codice - 80% in meno!
node Gain {
  -> in audio
  -> gain control = 1.0 @smooth
  <- out audio

  @process {
    out = in * gain
  }
}
```

**Vantaggi:**
- ✅ **80% meno codice** (11 vs 54 righe)
- ✅ **Zero variabili interne** da gestire
- ✅ **Automaticamente dual-mode** (audio-rate + control-rate)
- ✅ **Click-free garantito** senza sforzo
- ✅ **Focus su DSP** invece che su boilerplate

---

## Esempi Reali

### Volume Control con Slider

#### ❌ Prima (Complicato)
```microscript
node Volume {
  -> in audio
  -> volume control = 0.5
  <- out audio

  targetVolume = volume
  currentVolume = volume
  deltaVolume = 0
  needsSmoothing = false

  @prepare {
    currentVolume = volume
    targetVolume = volume
  }

  @control {
    if (volume != targetVolume) {
      targetVolume = volume
      needsSmoothing = true
    }
  }

  @process {
    if (needsSmoothing) {
      deltaVolume = (targetVolume - currentVolume) / samplesPerBlock
      needsSmoothing = false
    }
    out = in * currentVolume
    currentVolume += deltaVolume
    if (sampleIndex == samplesPerBlock - 1) {
      currentVolume = targetVolume
    }
  }
}
```

#### ✅ Dopo (Automatico)
```microscript
node Volume {
  -> in audio
  -> volume control = 0.5 @smooth @min(0) @max(1)
  <- out audio

  @process {
    out = in * volume
  }
}
```

---

### Low-Pass Filter con Cutoff

#### ❌ Prima
```microscript
node LowPass {
  -> in audio
  -> cutoff control = 1000.0
  <- out audio

  // Smoothing per cutoff
  targetCutoff = cutoff
  currentCutoff = cutoff
  deltaCutoff = 0
  needsSmoothingCutoff = false

  // Coefficienti del filtro
  a0 = 0
  b1 = 0

  @control {
    if (cutoff != targetCutoff) {
      targetCutoff = cutoff
      needsSmoothingCutoff = true
    }
  }

  @process {
    if (needsSmoothingCutoff) {
      deltaCutoff = (targetCutoff - currentCutoff) / samplesPerBlock
      needsSmoothingCutoff = false
    }
    
    // Calcola coefficienti con currentCutoff
    a0 = calculateA0(currentCutoff)
    b1 = calculateB1(currentCutoff)
    
    out = filter(in, a0, b1)
    
    currentCutoff += deltaCutoff
    if (sampleIndex == samplesPerBlock - 1) {
      currentCutoff = targetCutoff
    }
  }
}
```

#### ✅ Dopo
```microscript
node LowPass {
  -> in audio
  -> cutoff control = 1000.0 @smooth @min(20) @max(20000) @unit(Hz)
  <- out audio

  @process {
    // cutoff è già smoothato automaticamente!
    a0 = calculateA0(cutoff)
    b1 = calculateB1(cutoff)
    out = filter(in, a0, b1)
  }
}
```

---

### Panner (Stereo)

#### ❌ Prima
```microscript
node Panner {
  -> in audio
  -> pan control = 0.5
  <- left audio
  <- right audio

  targetPan = pan
  currentPan = pan
  deltaPan = 0
  needsSmoothingPan = false

  @control {
    if (pan != targetPan) {
      targetPan = pan
      needsSmoothingPan = true
    }
  }

  @process {
    if (needsSmoothingPan) {
      deltaPan = (targetPan - currentPan) / samplesPerBlock
      needsSmoothingPan = false
    }
    
    left = in * (1.0 - currentPan)
    right = in * currentPan
    
    currentPan += deltaPan
    if (sampleIndex == samplesPerBlock - 1) {
      currentPan = targetPan
    }
  }
}
```

#### ✅ Dopo
```microscript
node Panner {
  -> in audio
  -> pan control = 0.5 @smooth @min(0) @max(1)
  <- left audio
  <- right audio

  @process {
    left = in * (1.0 - pan)
    right = in * pan
  }
}
```

---

## Tremolo: Audio-Rate Transparente

Il sistema gestisce **automaticamente** anche audio-rate modulation:

```microscript
node Tremolo {
  -> carrier audio
  -> lfo audio  // Audio input invece di control
  <- out audio

  @process {
    // Nessuno smoothing quando lfo è audio-rate
    // Il sistema rileva automaticamente!
    out = carrier * lfo
  }
}

// Uso:
sine: SineOscillator { frequency=440 }
lfo: SineOscillator { frequency=4, amplitude=0.5, offset=0.5 }
tremolo: Tremolo { carrier=sine.out, lfo=lfo.out }
```

---

## Multipli Parametri Smoothati

### ❌ Prima: Esplosione Combinatoria
```microscript
node MultiParam {
  -> in audio
  -> gain control = 1.0
  -> cutoff control = 1000.0
  -> resonance control = 0.5
  <- out audio

  // 3 parametri = 12 variabili interne!
  targetGain = gain
  currentGain = gain
  deltaGain = 0
  needsSmoothingGain = false

  targetCutoff = cutoff
  currentCutoff = cutoff
  deltaCutoff = 0
  needsSmoothingCutoff = false

  targetResonance = resonance
  currentResonance = resonance
  deltaResonance = 0
  needsSmoothingResonance = false

  @control {
    if (gain != targetGain) {
      targetGain = gain
      needsSmoothingGain = true
    }
    if (cutoff != targetCutoff) {
      targetCutoff = cutoff
      needsSmoothingCutoff = true
    }
    if (resonance != targetResonance) {
      targetResonance = resonance
      needsSmoothingResonance = true
    }
  }

  @process {
    // Smoothing gain
    if (needsSmoothingGain) {
      deltaGain = (targetGain - currentGain) / samplesPerBlock
      needsSmoothingGain = false
    }
    // Smoothing cutoff
    if (needsSmoothingCutoff) {
      deltaCutoff = (targetCutoff - currentCutoff) / samplesPerBlock
      needsSmoothingCutoff = false
    }
    // Smoothing resonance
    if (needsSmoothingResonance) {
      deltaResonance = (targetResonance - currentResonance) / samplesPerBlock
      needsSmoothingResonance = false
    }
    
    // Processing
    out = filter(in * currentGain, currentCutoff, currentResonance)
    
    // Update all
    currentGain += deltaGain
    currentCutoff += deltaCutoff
    currentResonance += deltaResonance
    
    // Snap all
    if (sampleIndex == samplesPerBlock - 1) {
      currentGain = targetGain
      currentCutoff = targetCutoff
      currentResonance = targetResonance
    }
  }
}
```

### ✅ Dopo: Scale Linearmente
```microscript
node MultiParam {
  -> in audio
  -> gain control = 1.0 @smooth
  -> cutoff control = 1000.0 @smooth
  -> resonance control = 0.5 @smooth
  <- out audio

  @process {
    out = filter(in * gain, cutoff, resonance)
  }
}
```

**3 parametri smoothati:**
- Prima: ~70 righe di codice
- Dopo: ~10 righe di codice
- **Riduzione: 85%** 🎉

---

## Implementazione nel Compilatore

Quando il compilatore MicroScript vede:

```microscript
-> gain control = 1.0 @smooth
```

Genera automaticamente:

```cpp
class GeneratedNode : public Node {
private:
  // Parametro pubblico
  float gain_ = 1.0f;
  
  // Variabili interne generate automaticamente
  float targetGain_ = 1.0f;
  float currentGain_ = 1.0f;
  float deltaGain_ = 0.0f;
  bool needsSmoothingGain_ = false;

public:
  void setControlValue(const std::string& name, float value) override {
    if (name == "gain") {
      if (value != targetGain_) {
        targetGain_ = value;
        needsSmoothingGain_ = true;
      }
    }
  }

  void process(uint32_t samplesPerBlock) override {
    // Dual-mode automatico
    if (audioInputs[1] != nullptr) {
      // Audio-rate
      for (uint32_t i = 0; i < samplesPerBlock; i++) {
        audioOutputs[0][i] = audioInputs[0][i] * audioInputs[1][i];
      }
    } else {
      // Control-rate con smoothing
      if (needsSmoothingGain_) {
        deltaGain_ = (targetGain_ - currentGain_) / samplesPerBlock;
        needsSmoothingGain_ = false;
      }
      
      for (uint32_t i = 0; i < samplesPerBlock; i++) {
        audioOutputs[0][i] = audioInputs[0][i] * currentGain_;
        currentGain_ += deltaGain_;
      }
      
      currentGain_ = targetGain_;
      deltaGain_ = 0.0f;
    }
  }
};
```

---

## Conclusioni

### Statistiche di Semplificazione

| Esempio | Prima | Dopo | Riduzione |
|---------|-------|------|-----------|
| Gain semplice | 54 righe | 11 righe | **80%** |
| Volume control | 38 righe | 9 righe | **76%** |
| Low-pass filter | 42 righe | 12 righe | **71%** |
| Panner stereo | 35 righe | 11 righe | **69%** |
| Multi-param (3) | 70 righe | 10 righe | **85%** |

### Perché Questo È Importante

1. **Meno Codice = Meno Bug**
2. **Focus su DSP** invece che su boilerplate
3. **Esperienza Utente Migliore** (click-free garantito)
4. **Scalabilità** (10 parametri smoothati = stesse 10 righe!)
5. **Leggibilità** (il codice mostra l'intent, non i dettagli)

### Regola d'Oro

**Se un parametro è controllato da un utente (slider, knob, automation) → `@smooth`** ✅

Semplice, potente, automatico! 🎯
