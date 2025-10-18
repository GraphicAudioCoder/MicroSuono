# Automatic Parameter Smoothing in MicroScript

## Overview

MicroScript offre un sistema di **smoothing automatico** dei parametri che elimina i click e rende l'esperienza utente molto più semplice. L'utente non deve gestire manualmente `targetGain`, `currentGain`, `deltaGain` - tutto è gestito dal sistema!

## Basic Usage

### Semplice: Aggiungi `@smooth`

```microscript
node Gain {
  -> in audio
  -> gain control = 1.0 @smooth  // ✅ Smoothing automatico!
  <- out audio

  @process {
    out = in * gain  // Così semplice!
  }
}
```

### Senza Smoothing (Raw Control)

```microscript
node HardGate {
  -> in audio
  -> threshold control = 0.5  // ❌ No @smooth = cambiamenti istantanei
  <- out audio

  @process {
    out = (in > threshold) ? in : 0
  }
}
```

## Come Funziona Internamente

Quando il compilatore MicroScript vede `@smooth` su un parametro di controllo:

### 1. **Genera Variabili Interne Automaticamente**
```cpp
// L'utente scrive:
-> gain control = 1.0 @smooth

// Il compilatore genera:
float gain_ = 1.0;              // Valore pubblico (per l'utente)
float targetGain_ = 1.0;        // Target dello smoothing
float currentGain_ = 1.0;       // Valore corrente smoothato
float deltaGain_ = 0.0f;        // Delta per interpolazione
bool needsSmoothing_ = false;   // Flag per trigger
```

### 2. **Inietta Codice nel `@prepare`**
```cpp
@prepare {
  currentGain_ = gain_;
  targetGain_ = gain_;
  deltaGain_ = 0.0f;
}
```

### 3. **Inietta Controllo nel `@control`** (opzionale)
```cpp
@control {
  if (gain_ != targetGain_) {
    targetGain_ = gain_;
    needsSmoothing_ = true;
  }
}
```

### 4. **Trasforma il Codice nel `@process`**

L'utente scrive:
```microscript
out = in * gain
```

Il compilatore genera:
```cpp
// Check if audio-rate modulation
if (audioInputs[1] != nullptr) {
  // Audio-rate: direct multiplication
  audioOutputs[0][i] = audioInputs[0][i] * audioInputs[1][i];
} else {
  // Control-rate: smoothing
  if (needsSmoothing_) {
    deltaGain_ = (targetGain_ - currentGain_) / samplesPerBlock_;
    needsSmoothing_ = false;
  }
  
  audioOutputs[0][i] = audioInputs[0][i] * currentGain_;
  currentGain_ += deltaGain_;
  
  // Snap to target at block end
  if (i == samplesPerBlock_ - 1) {
    currentGain_ = targetGain_;
    deltaGain_ = 0.0f;
  }
}
```

## Dual-Mode Automatico

Il sistema gestisce **automaticamente** due modalità:

### Audio-Rate Modulation
```cpp
// LFO connesso al gain
auto lfo = std::make_shared<SineNode>("lfo", 4.0f);
graph.connect("lfo", "out", "gain_node", "gain");
// ✅ Il sistema rileva la connessione audio e bypassa lo smoothing!
```

### Control-Rate Smoothing
```cpp
// Slider GUI che cambia il gain
gainNode->setControlValue("gain", 0.8f);
// ✅ Il sistema applica smoothing JSFX-style automaticamente!
```

## Parametri Opzionali per `@smooth`

### Default (Raccomandato)
```microscript
-> gain control = 1.0 @smooth
// Usa samplesPerBlock automaticamente (tipicamente 512 samples @ 44.1kHz ≈ 11.6ms)
```

### Custom Time
```microscript
-> gain control = 1.0 @smooth(20ms)
// Smoothing su 20 millisecondi
```

### Instant Snap
```microscript
-> gain control = 1.0 @smooth(0ms)
// Equivalente a non avere @smooth, ma mantiene la dual-mode logic
```

## Pattern Comuni

### Slider di Volume
```microscript
node Volume {
  -> in audio
  -> volume control = 0.5 @smooth @min(0) @max(1)
  <- out audio
  
  @fadein(50ms)  // Smooth start
  @process {
    out = in * volume
  }
}
```

### Filtro con Cutoff Smoothato
```microscript
node LowPassFilter {
  -> in audio
  -> cutoff control = 1000.0 @smooth @min(20) @max(20000) @unit(Hz)
  <- out audio
  
  @fadein(30ms)
  @process {
    // coefficienti del filtro calcolati con cutoff smoothato
    // (cutoff viene aggiornato gradualmente)
    out = filter(in, cutoff)
  }
}
```

### Pan con Smoothing
```microscript
node Panner {
  -> in audio
  -> pan control = 0.5 @smooth @min(0) @max(1)  // 0=left, 1=right
  <- left audio
  <- right audio
  
  @fadein(40ms)
  @process {
    left = in * (1.0 - pan)
    right = in * pan
  }
}
```

### Tremolo (Audio-Rate, No Smoothing)
```microscript
node Tremolo {
  -> carrier audio
  -> lfo audio  // Audio-rate input
  <- out audio
  
  @fadein(0ms)  // No fade-in for modulators
  @process {
    // lfo è audio-rate, no smoothing applicato!
    out = carrier * lfo
  }
}
```

## Vantaggi del Sistema

### ✅ **Semplicità per l'Utente**
- Niente variabili interne da gestire
- Codice pulito e leggibile
- Focus sulla logica DSP, non sulla gestione dello stato

### ✅ **Click-Free Automatico**
- Tutti i parametri smoothati non generano click
- Interpolazione lineare JSFX-style su ogni blocco

### ✅ **Dual-Mode Trasparente**
- Supporta sia control-rate che audio-rate
- Il sistema rileva automaticamente la modalità
- L'utente non deve scrivere condizioni `if`

### ✅ **Performance**
- Smoothing solo quando necessario (`needsSmoothing_` flag)
- Zero overhead se il parametro non cambia
- Audio-rate bypass quando LFO è connesso

## Implementazione Manuale (Advanced)

Se l'utente vuole controllo totale, può implementare smoothing custom:

```microscript
node CustomGain {
  -> in audio
  -> gain control = 1.0  // No @smooth, gestione manuale
  <- out audio
  
  // Variabili custom
  myTarget = gain
  myCurrent = gain
  mySpeed = 0.01  // Smoothing più lento
  
  @process {
    // Smoothing exponenziale invece di lineare
    myCurrent = myCurrent + (myTarget - myCurrent) * mySpeed
    out = in * myCurrent
  }
}
```

## Conclusione

Il decoratore `@smooth` rende MicroScript **estremamente facile da usare** per creare interfacce utente con slider, knob e controlli senza preoccuparsi di click o artefatti. 

**Regola d'oro**: Se il parametro è controllato da un utente (slider, knob) → aggiungi `@smooth` ✅
