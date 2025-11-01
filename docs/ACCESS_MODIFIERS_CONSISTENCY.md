# Coerenza nei Modificatori di Accesso

## Data
1 novembre 2025

## Problemi Identificati e Risolti

### Principio Generale Applicato

**Per la classe base `Node`:**
- Membri che le sottoclassi devono leggere/modificare → **protected** (senza getter/setter per performance)
- Dati interni all'implementazione → **private** (con getter/setter pubblici solo se necessari per API pubblica)

**Per le classi derivate:**
- Stato interno estendibile da future sottoclassi → **protected** (per permettere ereditarietà)
- Parametri configurabili dall'utente → getter/setter pubblici per API chiara
- Stato puramente interno → **private** (solo se classe terminale)

### Modifiche Apportate

#### 1. `Node.hpp` (Classe Base)
**Prima:**
- `id_` e `params_` erano private (corretto - API pubblica)
- `inputPorts_`, `outputPorts_`, `sampleRate_`, `blockSize_` erano protected (corretto - usati dalle sottoclassi)
- Variabili fade-in erano **protected** (inconsistente - non servono alle sottoclassi)

**Dopo:**
```cpp
protected:
  std::vector<Port> inputPorts_;   // Sottoclassi devono accedervi
  std::vector<Port> outputPorts_;  
  int sampleRate_;                  // Sottoclassi lo leggono
  int blockSize_;                   
  class GraphManager* graphManager_;

private:
  // Fade-in (implementazione interna, non serve alle sottoclassi)
  float fadeInDurationMs_;
  int fadeInSamples_;
  int currentFadeSample_;
  bool fadeInActive_;
  
  std::string id_;                  // API pubblica via getId()
  std::vector<Param> params_;       // API pubblica via getParams()
```

**Motivazione:** Le sottoclassi chiamano `applyFadeIn()` ma non devono manipolare direttamente lo stato del fade-in. Questo incapsula meglio l'implementazione.

---

#### 2. `AudioInputNode.hpp`
**Prima:**
```cpp
private:
  int channelIndex_;
```

**Dopo:**
```cpp
protected:
  int channelIndex_;  // Sottoclassi potrebbero voler accedere all'indice
```

**Motivazione:** Una sottoclasse potrebbe voler implementare funzionalità diverse basate sul canale fisico (es. stereo input node).

---

#### 3. `GainNode.hpp`
**Prima:**
```cpp
private:
  float targetGain_;
  float currentGain_;
  float deltaGain_;
  bool needsSmoothing_;
  int samplesPerBlock_;
```

**Dopo:**
```cpp
protected:
  float targetGain_;      // Sottoclassi potrebbero voler estendere lo smoothing
  float currentGain_;
  float deltaGain_;
  bool needsSmoothing_;
  int samplesPerBlock_;
```

**Motivazione:** Potenziali estensioni come `FaderNode` o `VolumeEnvelopeNode` potrebbero voler accedere/modificare il meccanismo di smoothing.

---

#### 4. `SineOscillatorNode.hpp`
**Prima:** Tutti i membri `private`

**Dopo:** Tutti i membri `protected` con documentazione Doxygen

**Motivazione:** Permette di creare oscillatori derivati (es. `TriangleOscillator`, `SawtoothOscillator`) che condividono lo stesso meccanismo di smoothing e gestione della fase.

---

#### 5. `SineNode.hpp`
**Prima:** Tutti i membri `private`

**Dopo:** Tutti i membri `protected` con documentazione Doxygen

**Motivazione:** Come sopra - permette estensioni con diverse forme d'onda.

---

#### 6. `MixerNode.hpp`
**Prima:** Tutti i membri `private`

**Dopo:** Tutti i membri `protected` con documentazione Doxygen

**Motivazione:** Permette di creare mixer specializzati (es. `CrossfaderNode`, `SubmixNode`) che estendono la logica base.

---

#### 7. `MonoToStereoNode.hpp`
**Prima:**
```cpp
private:
  float pan_;
```

**Dopo:**
```cpp
protected:
  float pan_;
```

**Motivazione:** Una sottoclasse potrebbe voler implementare curve di panning diverse (es. constant-power panning).

---

#### 8. `ThresholdNode.hpp`
**Prima:** Membri `private`

**Dopo:** Membri `protected`

**Motivazione:** Permette di creare rivelatori di soglia specializzati (es. con isteresi, smoothing, ecc.).

---

#### 9. `SliderNode.hpp`
**Prima:** Membri `private` (corretto)

**Dopo:** Membri `private` con documentazione Doxygen migliorata

**Motivazione:** `SliderNode` è una classe terminale (UI control), difficilmente verrà estesa. I getter/setter pubblici forniscono già tutta l'API necessaria.

---

### Linee Guida per Futuri Nodi

1. **Membri protected:**
   - Stato che sottoclassi potrebbero voler leggere/modificare
   - Buffer interni, parametri di smoothing, stato dell'oscillatore
   - Sempre documentati con `///< Doxygen comment`

2. **Membri private:**
   - Implementazione completamente interna
   - Classi terminali (UI controls, utility nodes)
   - Dati esposti già tramite API pubblica (getter/setter)

3. **Getter/Setter pubblici:**
   - Solo per parametri che l'utente deve configurare
   - Non per ottimizzazione - le sottoclassi accedono direttamente ai protected
   - API chiara e documentata

4. **Performance:**
   - In realtime audio (metodo `process()`), accesso diretto ai membri protected
   - Nessun overhead di chiamata a getter/setter
   - Smoothing e interpolazione gestiti internamente

---

## Vantaggi della Nuova Organizzazione

1. **Estendibilità:** Le sottoclassi possono accedere allo stato necessario senza violare l'incapsulamento
2. **Performance:** Nessun overhead di getter/setter in codice realtime
3. **Chiarezza:** API pubblica ben definita tramite getter/setter, internals accessibili solo alla gerarchia
4. **Consistenza:** Pattern uniforme in tutte le classi di nodi
5. **Documentazione:** Tutti i membri protected sono documentati

---

## Test di Compilazione

Tutti i target compilano senza errori:
- `MicroSuonoLib` ✅
- Tutti i demo (demo1-demo11) ✅
- Tutti i test ✅

Nessun breaking change nell'API pubblica.
