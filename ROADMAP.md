# MicroSuono Development Roadmap

## Project Vision
Un ambiente di sviluppo audio modulare e real-time, alternativa a Max/MSP e SuperCollider, con:
- Architettura a nodi grafici interconnessi
- Linguaggio DSP custom per scripting nodi
- GUI visuale con ImGui + ImNodes
- Backend audio leggero e cross-platform (miniaudio)
- Thread-safe e real-time safe

---

## ✅ Fase 1: Architettura Core (COMPLETATA)

### Audio Engine & Graph
- ✅ Sistema di nodi con porte (Audio, Control, Event)
- ✅ GraphManager per gestione grafo e connessioni
- ✅ AudioEngine con supporto multicanale I/O
- ✅ Processamento real-time con miniaudio
- ✅ Event system per messaggi sample-accurate
- ✅ Supporto input/output N-canali

### Nodi Base Implementati
- ✅ SineNode (generatore sinusoide)
- ✅ GainNode (amplificazione/attenuazione)
- ✅ MonoToStereoNode (panning constant-power)
- ✅ ThresholdNode (detector con eventi)
- ✅ AudioInputNode (input da microfono/scheda)

### Documentazione
- ✅ Docs architetturali (Node, Port, GraphManager, AudioEngine)
- ✅ Esempi funzionanti (multichannel, input audio)

---

## 🎯 Fase 2: Linguaggio DSP Custom (PRIORITÀ MASSIMA)

### Obiettivo
Creare un linguaggio di scripting DSP espressivo, sicuro e real-time friendly per definire la logica dei nodi.

### 2.1 Design del Linguaggio

**Caratteristiche richieste:**
- Sintassi semplice e musicalmente intuitiva
- Operazioni sample-by-sample e per blocchi
- Type-safe (float, int, bool, array)
- Nessun garbage collector
- Nessuna allocazione dinamica in real-time
- Accesso diretto a input/output/parametri

**Esempio sintassi target:**
```
node Oscillator {
  inputs: freq (control)
  outputs: out (audio)
  state: phase (float)
  
  init {
    phase = 0.0
  }
  
  process {
    for sample in block {
      out[sample] = sin(phase)
      phase += 2*PI * freq / sampleRate
      if phase > 2*PI { phase -= 2*PI }
    }
  }
}
```

**Decisioni da prendere:**
1. Linguaggio interpretato o compilato?
   - **Interpretato**: Più flessibile, più lento
   - **Compilato (JIT)**: Più veloce, più complesso (LLVM?)
   - **Ibrido**: Compilato a bytecode, interpretato VM custom

2. Ispirazione da linguaggi esistenti?
   - Faust (funzionale, signal-oriented)
   - ChucK (temporale, event-driven)
   - SuperCollider (OOP, pattern-based)
   - Custom DSL da zero

**Raccomandazione: Inizia con un interprete bytecode custom**
- Più controllo sulla safety real-time
- Performance accettabile con ottimizzazioni
- Possibilità di JIT compilation in futuro

### 2.2 Componenti del Linguaggio

#### Lexer & Parser
- Tokenization del codice sorgente
- AST (Abstract Syntax Tree)
- Type checking statico

#### Compiler
- AST → Bytecode
- Ottimizzazioni (constant folding, dead code elimination)
- Validazione real-time safety

#### Virtual Machine
- Interprete bytecode veloce
- Stack-based o register-based
- Operazioni ottimizzate per DSP (SIMD quando possibile)

#### Runtime
- Binding con nodi C++
- Gestione stato del nodo
- Bridge per parametri e I/O

### 2.3 Libreria Standard DSP
- Oscillatori (sine, saw, square, triangle, noise)
- Filtri (lowpass, highpass, bandpass, notch)
- Envelope generators (ADSR, AR)
- Delay lines
- Math operations (clamp, map, interpolate)
- Array/buffer operations

---

## 🎨 Fase 3: GUI con ImGui + ImNodes

### 3.1 Node Editor Visuale
- Rendering del grafo con ImNodes
- Drag & drop per creare nodi
- Connessioni visuali tra porte
- Zoom, pan, minimap
- Selezione multipla, copy/paste

### 3.2 Editor di Codice Integrato
- Syntax highlighting per il linguaggio DSP
- Autocompletamento
- Error highlighting in tempo reale
- Hot reload (ricompila nodo senza fermare audio)

Librerie consigliate:
- ImGuiColorTextEdit (editor con syntax highlighting)
- Tree-sitter (parsing incrementale per highlighting)

### 3.3 Inspector/Properties Panel
- Visualizzazione parametri del nodo selezionato
- Slider, knob, input numerici
- Waveform/spectrum visualizer
- Level meters

### 3.4 Audio Scope & Analyzers
- Oscilloscopio real-time
- Spectrum analyzer (FFT)
- Level meters con peak hold
- Vectorscope (stereo)

---

## 🔧 Fase 4: Sistema di Nodi Esteso

### 4.1 Nodi Base Aggiuntivi
- **Generatori**: Saw, Square, Triangle, Noise, Wavetable
- **Filtri**: Biquad (LP/HP/BP/Notch), SVF, Comb
- **Dinamica**: Compressor, Limiter, Gate, Expander
- **Effetti**: Reverb, Delay, Chorus, Flanger, Phaser
- **Modulazione**: LFO, Envelope follower, Sample & Hold
- **Mixing**: Mixer N→1, Splitter 1→N, Crossfade
- **Utility**: DC Blocker, Clipper, Waveshaper, Quantizer

### 4.2 Nodi di Controllo
- MIDI Input/Output
- OSC (Open Sound Control)
- Clock/Sequencer
- Pattern generators

### 4.3 Nodi di Analisi
- Level meter
- Spectrum analyzer
- Pitch detector
- Onset detector

---

## 🎹 Fase 5: Controllo e Automazione

### 5.1 MIDI Support
- MIDI input node (note, CC, clock)
- MIDI output node
- MIDI mapping a parametri

### 5.2 OSC Support
- OSC receiver node
- OSC sender node
- Integrazione con controller esterni

### 5.3 Automation System
- Registrazione automazioni
- Curve editor
- Modulazione di parametri

---

## 💾 Fase 6: Persistence & Projects

### 6.1 Salvataggio/Caricamento
- Formato JSON per grafi
- Serializzazione stato nodi
- Preset system

### 6.2 Project Management
- File .msproj (graph + settings)
- Asset manager (samples, presets)
- Version control friendly format

---

## 🚀 Fase 7: Advanced Features

### 7.1 Multi-threading
- Parallel node processing (ove possibile)
- Lock-free queues per events

### 7.2 Plugin System
- Dynamic loading di nodi custom (DLL/SO)
- VST3/AU wrapper (esporta nodo come plugin)

### 7.3 Collaborative Features
- Network streaming
- Remote control via web interface

---

## 📋 Priorità Immediate (Prossimi Steps)

### Step 1: Prototipo Linguaggio DSP (2-3 settimane)
1. Design sintassi base (minimal viable syntax)
2. Lexer semplice (tokenization)
3. Parser → AST
4. Interprete diretto AST (no bytecode ancora)
5. Binding con Node C++
6. Test: creare SineNode in DSL

### Step 2: Primi Nodi in DSL (1 settimana)
1. Riscrivere SineNode in DSL
2. Creare SawNode, SquareNode
3. Creare FilterNode (biquad LP)

### Step 3: GUI Minima (2 settimane)
1. Setup ImGui + ImNodes
2. Visualizzare grafo esistente
3. Creare/eliminare nodi con UI
4. Connettere nodi con mouse

### Step 4: Editor Integrato (1-2 settimane)
1. Integrare ImGuiColorTextEdit
2. Syntax highlighting base per DSL
3. Compilazione on-the-fly

---

## 🧪 Da Cosa Partire?

### ⭐ CONSIGLIO: Inizia dal Linguaggio DSL

**Perché:**
1. È il cuore del sistema: determina come gli utenti creano contenuti
2. Influenza architettura futura (VM, hot reload, debugging)
3. Puoi testarlo con nodi esistenti (nessuna GUI necessaria)
4. Validazione early delle scelte di design

**Piano d'attacco:**
1. **Settimana 1-2**: Design sintassi + parser minimale
2. **Settimana 3**: Interprete AST + binding con Node
3. **Settimana 4**: Test con 3-4 nodi base (Osc, Filter, Gain)
4. **Settimana 5**: Ottimizzazioni + bytecode VM (opzionale)

Una volta solido il linguaggio, la GUI diventa "solo" una visualizzazione del grafo e un editor di testo con syntax highlighting.

---

## 🛠️ Tools & Librerie Consigliate

### Parsing
- **Antlr4**: Parser generator (overkill ma potente)
- **Custom lexer/parser**: Massimo controllo
- **PEG (Parsing Expression Grammar)**: Semplice e potente

### JIT Compilation (futuro)
- **LLVM**: Industry standard, complesso
- **TinyCC**: Leggero, più semplice
- **Custom VM**: Massimo controllo

### GUI
- **ImGui**: Immediate mode, perfetto per tool
- **ImNodes**: Node editor già pronto
- **ImGuiColorTextEdit**: Code editor integrato

### Audio DSP
- **JUCE modules**: Solo DSP, senza GUI
- **Gamma**: Lightweight DSP library
- **Custom**: Massimo controllo (raccomandato)

---

## 📚 Risorse Utili

### Linguaggi DSP Esistenti (ispirazione)
- **Faust**: Funzionale, signal-oriented
- **ChucK**: Temporale, educativo
- **SuperCollider**: OOP, potente
- **Max/MSP Gen~**: Visual → code
- **Reaktor Core**: Low-level blocks

### Libri
- "Audio Programming Book" (MIT Press)
- "Designing Sound" (Andy Farnell, Pure Data)
- "Crafting Interpreters" (Bob Nystrom)

### Papers
- "Faust: Functional Audio Stream" (Orlarey et al.)
- "The SuperCollider Book" (MIT Press)

---

## 🎯 Success Metrics

### MVP (Minimum Viable Product)
- [ ] 10+ nodi base funzionanti in DSL
- [ ] GUI per creare/connettere nodi
- [ ] Editor codice con syntax highlighting
- [ ] Salvataggio/caricamento grafi
- [ ] Esempio funzionante: synth monofonico controllato

### v1.0
- [ ] 50+ nodi libreria standard
- [ ] Hot reload codice nodi
- [ ] MIDI input/output
- [ ] Automation system base
- [ ] Preset manager
- [ ] Tutorial e documentazione completa

---

## 🚦 Conclusione

**Sei pronto!** L'architettura audio è solida. Il prossimo step critico è il **linguaggio DSL**. Una volta definito, tutto il resto fluisce naturalmente.

**Focus primario: Settimane 1-4**
→ Design e implementazione del linguaggio DSL
→ Prototipo funzionante con 3-4 nodi

**Then:**
→ GUI (visualizzazione + editor)
→ Libreria nodi standard
→ Features avanzate

Buon lavoro! 🎵🚀
