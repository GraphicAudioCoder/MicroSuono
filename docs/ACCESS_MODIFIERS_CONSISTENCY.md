# Coerenza nei Modificatori di Accesso - Logica Definitiva

## Data
1 novembre 2025

---

## LA REGOLA FONDAMENTALE (Semplice e Non Ambigua)

```
PUBLIC    = API esterna (GraphManager, utente, linguaggio DSL)
PROTECTED = Solo metodi helper che sottoclassi DEVONO chiamare + dati diretti per performance
PRIVATE   = Tutto ciò che è implementazione nascosta
```

### Principio Chiave:
**Se una sottoclasse può accedere direttamente a un membro protected, NON serve un metodo getter/setter protected.**

---

## ANALISI Node.hpp - Prima e Dopo

### ❌ PROBLEMI ORIGINALI (Inconsistenti)

1. **Ridondanza inutile:**
   ```cpp
   // PROTECTED: Sia il dato che i metodi per manipolarlo
   std::vector<Port> inputPorts_;
   void addInputPort(...) { inputPorts_.push_back(...); }
   
   // Ma allora perché addInputPort()? La sottoclasse può fare push_back direttamente!
   ```

2. **Getter pubblici per dati già protected:**
   ```cpp
   public:
     const std::vector<Port>& getInputPorts() const { return inputPorts_; }
   protected:
     std::vector<Port> inputPorts_;  // Già accessibile!
   ```
   → **Incoerente:** GraphManager usa il getter pubblico, le sottoclassi accedono direttamente.

3. **Metodo protected con stato private:**
   ```cpp
   protected:
     void applyFadeIn(float* buffer, int nFrames) { /* usa fadeInActive_ */ }
   private:
     bool fadeInActive_;  // La sottoclasse NON può modificarlo
   ```
   → **Corretto:** La sottoclasse può solo chiamare `applyFadeIn()`, non manipolare lo stato.

4. **Wrapper inutile:**
   ```cpp
   protected:
     const float* getPhysicalInput(int i) const { return graphManager_->getPhysicalInput(i); }
     GraphManager* graphManager_;  // Già accessibile!
   ```
   → **Domanda:** Perché non `graphManager_->getPhysicalInput(i)` direttamente?
   → **Risposta:** Encapsulation - se cambia come si ottiene l'input, cambia solo Node.cpp.

---

## ✅ LOGICA CORRETTA APPLICATA

### PUBLIC (API Esterna)

```cpp
public:
  // === LIFECYCLE ===
  Node(const std::string& id);
  virtual ~Node() = default;
  
  // === API UTENTE (configurazione) ===
  const std::string& getId() const;                    // Lettura ID
  const std::vector<Param>& getParams() const;         // Lettura parametri
  std::vector<Param>& getParams();                     // Modifica parametri
  bool setParam(const std::string& name, ...);         // Set singolo parametro
  const ControlValue* getParam(const std::string& name) const;
  
  void setFadeInDuration(float durationMs);            // Configurazione fade-in
  float getFadeInDuration() const;
  void resetFadeIn();
  
  // === API GRAPHMANAGER (informazioni nodo) ===
  const std::vector<Port>& getInputPorts() const;     // Lettura porte input
  const std::vector<Port>& getOutputPorts() const;    // Lettura porte output
  void setGraphManager(GraphManager* graph);           // Collegamento al grafo
  
  // === PROCESSING (sottoclassi DEVONO implementare) ===
  virtual void prepare(int sampleRate, int blockSize);
  virtual void process(...) = 0;                       // PURE VIRTUAL
  virtual void processControl(...) {}                  // Opzionale
  virtual void processEvents(...) {}                   // Opzionale
```

**Regola PUBLIC:**
- Metodi che **GraphManager** o **utente finale** chiamano
- Getter per dati private (id, params)
- Getter per dati protected **solo se GraphManager ne ha bisogno** (inputPorts, outputPorts)

---

### PROTECTED (Per Sottoclassi)

```cpp
protected:
  // === METHODS: Solo helper che sottoclassi DEVONO chiamare ===
  
  void applyFadeIn(float* buffer, int nFrames);
  // Perché protected? La sottoclasse non può implementarlo (stato private)
  // Quando chiamare? Alla fine di process() sul buffer di output
  
  void addInputPort(const std::string& name, PortType type);
  void addOutputPort(const std::string& name, PortType type);
  // Perché protected? Convention chiara nel costruttore
  // Alternativa sarebbe: inputPorts_.push_back(Port(...))
  // Pro: Più leggibile, futureproof se cambia implementazione
  
  const float* getPhysicalInput(int channelIndex) const;
  // Perché protected? Encapsula graphManager_->getPhysicalInput()
  // Alternativa: graphManager_->getPhysicalInput(i) direttamente
  // Pro: Se cambia implementazione, tocchi solo Node.cpp
  
  // === DATA: Accesso diretto per performance in process() ===
  
  std::vector<Port> inputPorts_;   // Lettura consigliata, modifica via addInputPort()
  std::vector<Port> outputPorts_;  // Lettura consigliata, modifica via addOutputPort()
  
  int sampleRate_;    // Sottoclassi leggono per calcoli (es. phaseIncrement)
  int blockSize_;     // Sottoclassi leggono per buffer allocation
  
  GraphManager* graphManager_;  // Per getPhysicalInput() se serve
```

**Regola PROTECTED:**
- **Metodi:** Solo se la sottoclasse **non può implementarli** (serve accesso a dati private)
- **Dati:** Solo se la sottoclasse **deve leggerli/modificarli** (performance in process())
- **NO getter/setter protected** se il dato è già protected!

---

### PRIVATE (Implementazione Nascosta)

```cpp
private:
  // === Dati esposti tramite API pubblica ===
  std::string id_;              // Getter: getId()
  std::vector<Param> params_;   // Getter: getParams()
  
  // === Implementazione interna (sottoclassi non toccano) ===
  float fadeInDurationMs_;      // Configurazione fade-in
  int fadeInSamples_;           // Campioni di fade-in
  int currentFadeSample_;       // Posizione corrente
  bool fadeInActive_;           // Flag attivo
  
  void updateFadeInSamples();   // Helper interno
```

**Regola PRIVATE:**
- Dati che le sottoclassi **non devono** leggere/modificare
- Implementazione interna di feature (es. fade-in gestito da applyFadeIn())
- Dati esposti tramite getter/setter pubblici (id, params)

---

## LOGICA APPLICATA ALLE SOTTOCLASSI

### Esempio: GainNode

```cpp
class GainNode : public Node {
public:
  GainNode(const std::string& id, float gain = 1.0f);
  
  void prepare(int sampleRate, int blockSize) override;
  void process(...) override;
  void processControl(...) override;
  
  // API pubblica per utente
  void setGain(float gain);
  float getGain() const;

protected:
  // Dati che future sottoclassi potrebbero estendere
  // Es: AutoGainNode, CompressorNode potrebbero leggere/modificare
  float targetGain_;
  float currentGain_;
  float deltaGain_;
  bool needsSmoothing_;
  int samplesPerBlock_;
};
```

**Perché protected e non private?**
- Una sottoclasse `FaderNode` potrebbe voler modificare `targetGain_` con curve diverse
- Una sottoclasse `CompressorNode` potrebbe voler leggere `currentGain_` per threshold
- Se fossero private, queste estensioni sarebbero impossibili

**Quando usare private nelle sottoclassi?**
- Solo se la classe è **terminale** (non verrà mai ereditata)
- Esempio: `SliderNode` è un controllo UI, difficilmente verrà esteso

---

## CONFRONTO: Prima vs Dopo

### PRIMA (Inconsistente)

```cpp
// Ridondanza: dato protected + getter pubblico
public:
  const std::vector<Port>& getInputPorts() const { return inputPorts_; }
protected:
  std::vector<Port> inputPorts_;

// Ridondanza: dato protected + metodo helper protected
protected:
  void addInputPort(...) { inputPorts_.push_back(...); }
  std::vector<Port> inputPorts_;

// Inconsistenza: metodo protected, stato private
protected:
  void applyFadeIn(...) { /* usa fadeInActive_ */ }
private:
  bool fadeInActive_;
```

### DOPO (Coerente)

```cpp
// Getter pubblico: GraphManager legge le porte
public:
  const std::vector<Port>& getInputPorts() const { return inputPorts_; }

// Metodo helper: convenzione chiara, futureproof
protected:
  void addInputPort(...) { inputPorts_.push_back(...); }
  std::vector<Port> inputPorts_;  // Accesso diretto per performance

// Encapsulation: sottoclasse chiama metodo, non tocca stato
protected:
  void applyFadeIn(...);
private:
  bool fadeInActive_;  // Stato interno, gestito da applyFadeIn()
```

---

## DECISIONI DI DESIGN SPIEGATE

### 1. Perché `addInputPort()` è protected e non private?

**Opzione A (private):**
```cpp
class SineNode : public Node {
  SineNode(const std::string& id) : Node(id) {
    inputPorts_.push_back(Port("in", PortType::Audio));  // Accesso diretto
  }
};
```

**Opzione B (protected helper):**
```cpp
class SineNode : public Node {
  SineNode(const std::string& id) : Node(id) {
    addInputPort("in", PortType::Audio);  // Metodo helper
  }
};
```

**Scelta: Opzione B**
- Più leggibile e dichiarativo
- Futureproof: se cambia implementazione (es. validazione, logging), cambia solo Node.cpp
- Convenzione chiara in tutto il codebase

---

### 2. Perché `getPhysicalInput()` è protected?

**Opzione A (accesso diretto):**
```cpp
class AudioInputNode : public Node {
  void process(...) override {
    const float* input = graphManager_->getPhysicalInput(channelIndex_);
  }
};
```

**Opzione B (metodo wrapper):**
```cpp
class AudioInputNode : public Node {
  void process(...) override {
    const float* input = getPhysicalInput(channelIndex_);
  }
};
```

**Scelta: Opzione B**
- Encapsulation: se cambia come si ottiene l'input (es. caching, ring buffer), cambia solo Node.cpp
- Meno dipendenze: AudioInputNode non deve conoscere GraphManager
- Più testabile: puoi mockare getPhysicalInput() senza GraphManager

---

### 3. Perché `sampleRate_` e `blockSize_` sono protected?

```cpp
class SineOscillatorNode : public Node {
protected:
  void updatePhaseIncrement() {
    phaseIncrement_ = 2.0f * M_PI * frequency_ / sampleRate_;  // Legge sampleRate_
  }
};
```

**Alternativa (getter):**
```cpp
phaseIncrement_ = 2.0f * M_PI * frequency_ / getSampleRate();
```

**Scelta: Accesso diretto**
- Performance: `getSampleRate()` è una chiamata di funzione (anche se inline)
- Convenienza: tutti i nodi audio devono leggere sampleRate
- Semantica: `sampleRate_` è configuration, non implementation detail

---

## PROBLEMI IDENTIFICATI E RISOLTI

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
