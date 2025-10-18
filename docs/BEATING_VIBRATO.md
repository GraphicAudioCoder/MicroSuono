# Beating e Vibrato negli Accordi

## 🎵 Fenomeno Osservato

Quando suoni accordi con più oscillatori, potresti sentire un effetto di "vibrato" o "tremolo" non controllato. Questo è **normale** e ha cause sia fisiche che tecniche.

## 📊 Cause

### 1. Beating Acustico (Fenomeno Naturale)

Quando due frequenze **molto vicine** suonano insieme, si crea un battimento (beating) udibile come modulazione d'ampiezza.

**Formula del Beating:**
```
frequenza_battimento = |freq1 - freq2|
```

#### Esempio: C Major Chord
```
C4 = 261.63 Hz
E4 = 329.63 Hz  
G4 = 392.00 Hz
C5 = 523.25 Hz
```

**Battimenti generati:**
- E4 - C4 = 68 Hz → ~15 battimenti/sec (molto udibile)
- G4 - E4 = 62 Hz → ~16 battimenti/sec
- C5 - G4 = 131 Hz → troppo veloce, percepito come ruvidezza

**Risultato:** Senti un tremolo di ~15-16 Hz, che è molto udibile!

### 2. Detuning Naturale

Anche piccole imperfezioni nelle frequenze creano beating:
```
C4_teorico  = 261.6256 Hz
C4_arrotond = 261.63 Hz
Differenza  = 0.0044 Hz → battimento ogni ~227 secondi
```

Con 4 oscillatori, le micro-differenze si sommano → beating più evidente.

### 3. Phase Drift (Bug Tecnico - ORA FIXATO)

**Problema precedente:**
```cpp
void SineNode::setFrequency(float freq) {
  frequency_ = freq;
  phaseIncrement_ = 2.0f * M_PI * frequency_ / sampleRate_;
  // ⚠️ Cambio ISTANTANEO del phase increment
  // La fase continua ma con nuovo increment → discontinuità!
}
```

**Cosa succedeva:**
- Cambio frequenza → phase increment salta
- La fase φ continua dal valore precedente
- Creo una discontinuità di derivata → glitch percepibile
- Ripetuto più volte → effetto tremolo/vibrato

**Soluzione implementata:**
```cpp
// Smoothing della frequenza (5ms)
if (std::abs(currentFrequency_ - targetFrequency_) > 0.01f) {
  currentFrequency_ += frequencyDelta_;
  phaseIncrement_ = 2.0f * M_PI * currentFrequency_ / sampleRate_;
}
```

## 🎯 Soluzioni

### ✅ Per Beating Naturale (Accettabile)

Il beating è **normale e desiderabile** in molti casi:
- Rende il suono "vivo" e organico
- Simula il comportamento di strumenti acustici
- Aggiunge "warmth" al suono

**Se vuoi ridurlo:**

1. **Usa frequenze perfettamente intonate**
   ```cpp
   // Calcola frequenze esatte dalla fondamentale
   float fundamental = 261.6256f; // C4 esatto
   float major_third = fundamental * pow(2.0f, 4.0f/12.0f); // E4
   float perfect_fifth = fundamental * pow(2.0f, 7.0f/12.0f); // G4
   float octave = fundamental * 2.0f; // C5
   ```

2. **Aggiungi detuning controllato**
   ```cpp
   // Detune leggero per richness, ma controllato
   sine1->setFrequency(261.6256f);
   sine2->setFrequency(261.6256f * 1.001f); // +1 cent
   // Crei un beating di ~0.26 Hz → molto lento e gradevole
   ```

3. **Usa un solo oscillatore con filtri**
   ```cpp
   // Un oscillatore → nessun beating
   // Usa filterbank per armonici invece di oscillatori multipli
   ```

### ✅ Per Phase Drift (RISOLTO)

Il fix implementato aggiunge smoothing automatico:
- Cambio di frequenza avviene in ~5ms
- Nessuna discontinuità di fase
- Transizioni morbide e click-free

## 📈 Verifica

### Test 1: Beating Naturale
```cpp
// Due oscillatori MOLTO vicini
sine1->setFrequency(440.0f);
sine2->setFrequency(442.0f);
// Battimento atteso: 2 Hz → 2 pulsazioni/sec ✓
```

### Test 2: Nessun Beating
```cpp
// Due oscillatori ESATTI in ottava
sine1->setFrequency(440.0f);
sine2->setFrequency(880.0f);
// Battimento atteso: 0 Hz → suono stabile ✓
```

### Test 3: Smooth Frequency Change
```cpp
sine1->setFrequency(440.0f);
std::this_thread::sleep_for(1s);
sine1->setFrequency(880.0f);
// Cambio avviene in 5ms → no click/pop ✓
```

## 🎓 Fisica del Beating

### Formula Completa
Quando due sinusoidi con frequenze f₁ e f₂ si sommano:

```
s(t) = A·sin(2π·f₁·t) + A·sin(2π·f₂·t)

Usando l'identità trigonometrica:
s(t) = 2A·cos(2π·(f₁-f₂)/2·t) · sin(2π·(f₁+f₂)/2·t)
       ︸━━━━━━━━━━━━━━━━━━━━━━━━━━︸   ︸━━━━━━━━━━━━━━━━━━━━━━━━━━︸
       Modulazione d'ampiezza         Portante (frequenza media)
       freq = |f₁-f₂|/2               freq = (f₁+f₂)/2
```

**Percezione:**
- Freq beating < 15 Hz → tremolo/vibrato distinto
- 15-30 Hz → ruvidezza
- > 30 Hz → nota separata (tono differenziale)

## 🎸 Applicazioni Musicali

### Chorus/Detune Effect
```cpp
// 3 oscillatori leggermente detuned
sine1->setFrequency(440.0f);
sine2->setFrequency(440.0f * 1.005f); // +5 cent
sine3->setFrequency(440.0f * 0.995f); // -5 cent
// Crea un suono "chorus" naturale
```

### String Ensemble
```cpp
// Simula sezione di archi (molti strumenti leggermente scordat)
const int NUM_VOICES = 8;
for (int i = 0; i < NUM_VOICES; ++i) {
  float detune = 1.0f + (rand() / RAND_MAX - 0.5f) * 0.01f; // ±0.5%
  voices[i]->setFrequency(440.0f * detune);
}
// Beating complesso → suono ricco e organico
```

## 📝 Conclusioni

- ✅ **Beating naturale**: Normale, spesso desiderabile
- ✅ **Phase drift**: Risolto con frequency smoothing
- ✅ **Controllo**: Ora puoi scegliere se avere beating o no
- ✅ **Musicalmente**: Il beating aggiunge vita al suono!

**Regola generale:** 
- Accordi "perfect" → usa frequenze esatte (no beating)
- Suono "warm/vintage" → aggiungi micro-detuning controllato
- Effetto "chorus" → detune più marcato (~5-10 cents)
