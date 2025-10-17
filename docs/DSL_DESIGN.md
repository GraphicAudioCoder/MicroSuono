# MicroSuono DSL - Design & Implementation

## Visione

Un linguaggio di scripting DSP semplice, espressivo e **veloce come C++** per creare nodi audio con GUI integrata, esportabili come VST3.

---

## Obiettivi

1. ✅ **Semplice**: Niente header, puntatori, gestione memoria
2. ✅ **Veloce**: Performance identiche a C++ (zero overhead runtime)
3. ✅ **Espressivo**: Sample-by-sample, control-rate, event-driven
4. ✅ **Type-safe**: Errori a compile-time, non a runtime
5. ✅ **GUI integrata**: Definisci controlli nel codice
6. ✅ **VST3-ready**: Export automatico in plugin

---

## Sintassi del Linguaggio

### Esempio Completo: Filtro con GUI

```dsl
node DynamicFilter {
  // === PORTS ===
  audio in, out;
  control cutoff_control;
  event reset;
  
  // === PARAMETERS (GUI-exposed) ===
  param cutoff = 1000.0 {
    min: 20.0,
    max: 20000.0,
    default: 1000.0,
    unit: "Hz",
    scale: "log"
  };
  
  param resonance = 0.7 {
    min: 0.1,
    max: 1.0,
    default: 0.7
  };
  
  // === INTERNAL STATE ===
  state {
    float z1 = 0.0;
    float z2 = 0.0;
    float currentCutoff = 1000.0;
  };
  
  // === GUI DEFINITION ===
  gui {
    layout: vertical;
    knob(cutoff, label: "Cutoff", size: large);
    knob(resonance, label: "Q");
    meter(out, orientation: vertical);
  };
  
  // === PROCESSING ===
  
  // Control-rate (once per block)
  onControl() {
    if (cutoff_control.isConnected()) {
      currentCutoff = cutoff_control.value;
    } else {
      currentCutoff = cutoff;
    }
  }
  
  // Event handling (sample-accurate)
  onEvent(reset) {
    z1 = 0.0;
    z2 = 0.0;
  }
  
  // Audio-rate (sample-by-sample)
  process(sample) {
    float freq = currentCutoff / sampleRate;
    float feedback = resonance + resonance / (1.0 - freq);
    
    z1 += freq * (in - z1 + feedback * (z1 - z2));
    z2 += freq * (z1 - z2);
    
    out = z2;
  }
}
```

---

## Type System

### Port Types

| Type | Description | Rate | C++ Equivalent |
|------|-------------|------|----------------|
| `audio` | Audio signal | Sample-rate (44100 Hz) | `float buffer[blockSize]` |
| `control` | Control signal | Block-rate (~86 Hz @ 512 samples) | `float value` |
| `event` | Discrete message | Sample-accurate | `Event{value, sampleOffset}` |

### Data Types

| Type | Description | Example |
|------|-------------|---------|
| `float` | 32-bit floating point | `1.5`, `440.0` |
| `int` | 32-bit integer | `0`, `127` |
| `bool` | Boolean | `true`, `false` |
| `string` | String literal | `"sine"`, `"Hz"` |
| `array` | Fixed-size array | `float buffer[8]` |

### Variable Scopes

| Keyword | Scope | Persistence | GUI-exposed |
|---------|-------|-------------|-------------|
| `param` | Node parameter | Yes (saved) | **Yes** |
| `state` | Internal state | Yes | No |
| `local` | Function local | No | No |

---

## Processing Callbacks

### Lifecycle

```
Node Creation
    ↓
init()          // Called once at creation
    ↓
prepare()       // Called when sample rate/block size changes
    ↓
┌─────────────────────────────────────┐
│  Audio Loop (real-time thread)      │
│                                      │
│  onControl()     // Once per block   │
│      ↓                               │
│  onEvent(e)      // For each event   │
│      ↓                               │
│  process(s)      // For each sample  │
│      ↓                               │
│  (repeat)                            │
└─────────────────────────────────────┘
    ↓
cleanup()       // Called on destruction
```

### Callback Details

| Callback | Called When | Use Case |
|----------|-------------|----------|
| `init()` | Node creation | Initialize state, allocate buffers |
| `prepare()` | Sample rate change | Recalculate coefficients |
| `onControl()` | Every block (before audio) | Update control-rate parameters |
| `onEvent(name)` | Event arrives | Handle triggers, notes, etc. |
| `process(sample)` | Every sample | DSP processing |
| `onChange(param)` | Parameter changes | React to GUI changes |

---

## Built-in Functions & Constants

### Math Functions

```dsl
// Trigonometry
sin(x), cos(x), tan(x)
asin(x), acos(x), atan(x), atan2(y, x)

// Exponential
exp(x), log(x), log10(x), pow(x, y), sqrt(x)

// Utility
abs(x), min(x, y), max(x, y)
clamp(x, min, max)
floor(x), ceil(x), round(x)
```

### Audio Utilities

```dsl
// Unit conversions
db_to_gain(db)        // 10^(db/20)
gain_to_db(gain)      // 20*log10(gain)
midi_to_freq(note)    // 440 * 2^((note-69)/12)
freq_to_midi(freq)    // 69 + 12*log2(freq/440)

// Interpolation
lerp(a, b, t)         // Linear: a + (b-a)*t
```

### Global Constants

```dsl
sampleRate    // Current sample rate (e.g., 44100.0)
blockSize     // Current block size (e.g., 512)
PI            // 3.14159265358979323846
TWO_PI        // 2*PI
E             // 2.71828182845904523536
```

---

## GUI Widgets

### Widget Types

| Widget | Parameters | Example |
|--------|-----------|---------|
| `knob` | `(param, label, size, style)` | Rotary knob |
| `slider` | `(param, label, orientation)` | Linear slider |
| `toggle` | `(param, label)` | On/off switch |
| `button` | `(event, label)` | Momentary button |
| `dropdown` | `(param, label, options)` | Option selector |
| `meter` | `(audio/control, orientation)` | Level meter |
| `scope` | `(audio, style)` | Oscilloscope |
| `led` | `(event/bool, color)` | Indicator light |
| `display` | `(value, format)` | Numeric readout |
| `xy_pad` | `(paramX, paramY)` | 2D controller |
| `curve_editor` | `(params..., style)` | Visual curve editing |

### Layout Options

```dsl
gui {
  layout: vertical;       // Stack vertically
  layout: horizontal;     // Stack horizontally
  layout: grid(cols, rows); // Grid layout
  layout: compact;        // Minimize spacing
  layout: minimal;        // Single control
}
```

---

## Compilation Strategy

### Obiettivo: **Performance C++ con Sintassi Semplice**

Il linguaggio DSL viene **compilato direttamente a C++** (non interpretato!) per ottenere performance identiche.

---

## Catena di Compilazione

```
┌─────────────────────────────────────────────────────────────────┐
│  FASE 1: PARSING                                                │
└─────────────────────────────────────────────────────────────────┘

  MyFilter.dsl (source code)
       ↓
  ┌─────────┐
  │ Lexer   │  Tokenization
  └─────────┘
       ↓
  Tokens: [KEYWORD(node), ID(MyFilter), LBRACE, ...]
       ↓
  ┌─────────┐
  │ Parser  │  Syntax analysis
  └─────────┘
       ↓
  AST (Abstract Syntax Tree)
  
  NodeDecl {
    name: "MyFilter"
    ports: [AudioPort("in"), AudioPort("out")]
    params: [ParamDecl("cutoff", 1000.0, ...)]
    state: [StateDecl("z1", float, 0.0)]
    callbacks: [
      onControl() { ... },
      process() { ... }
    ]
    gui: GUIDecl { ... }
  }

┌─────────────────────────────────────────────────────────────────┐
│  FASE 2: SEMANTIC ANALYSIS                                      │
└─────────────────────────────────────────────────────────────────┘

  AST
       ↓
  ┌──────────────┐
  │ Type Checker │  Verify types, connections
  └──────────────┘
       ↓
  Typed AST + Symbol Table
       ↓
  ┌──────────────┐
  │ Validator    │  Check real-time safety
  └──────────────┘
       ↓
  Validated AST
  
  Checks:
  ✓ No dynamic allocation in process()
  ✓ No I/O operations in callbacks
  ✓ Port types match (audio→audio, control→control)
  ✓ Array bounds are constant
  ✓ All parameters have valid ranges

┌─────────────────────────────────────────────────────────────────┐
│  FASE 3: OPTIMIZATION                                           │
└─────────────────────────────────────────────────────────────────┘

  Validated AST
       ↓
  ┌──────────────┐
  │ Optimizer    │  High-level optimizations
  └──────────────┘
       ↓
  Optimized AST
  
  Optimizations:
  • Constant folding: 2*PI → 6.283185...
  • Dead code elimination
  • Common subexpression elimination
  • Loop unrolling (small fixed loops)
  • Inline small functions

┌─────────────────────────────────────────────────────────────────┐
│  FASE 4: CODE GENERATION                                        │
└─────────────────────────────────────────────────────────────────┘

  Optimized AST
       ↓
  ┌──────────────────┐
  │ C++ Code Gen     │  Generate C++ source
  └──────────────────┘
       ↓
  Generated C++ Files:
  
  MyFilter.hpp        // Header (class declaration)
  MyFilter.cpp        // Implementation
  MyFilter_gui.cpp    // GUI generation code (ImGui)
  
  Example generated code:
  
  class MyFilter : public ms::Node {
  private:
    float param_cutoff = 1000.0f;
    float param_resonance = 0.7f;
    float state_z1 = 0.0f;
    float state_z2 = 0.0f;
    float state_currentCutoff = 1000.0f;
    
  public:
    void processControl() override {
      const float* cutoff_control_in = getControlInput("cutoff_control");
      if (cutoff_control_in) {
        state_currentCutoff = *cutoff_control_in;
      } else {
        state_currentCutoff = param_cutoff;
      }
    }
    
    void process(int numFrames) override {
      const float* in = getAudioInput(0);
      float* out = getAudioOutput(0);
      
      for (int i = 0; i < numFrames; ++i) {
        float freq = state_currentCutoff / sampleRate_;
        float feedback = param_resonance + param_resonance / (1.0f - freq);
        
        state_z1 += freq * (in[i] - state_z1 + feedback * (state_z1 - state_z2));
        state_z2 += freq * (state_z1 - state_z2);
        
        out[i] = state_z2;
      }
    }
    
    void renderGUI() override {
      ImGui::BeginVertical();
      ImGui::Knob("Cutoff", &param_cutoff, 20.0f, 20000.0f, ImGuiKnobFlags_Logarithmic);
      ImGui::Knob("Q", &param_resonance, 0.1f, 1.0f);
      ImGui::LevelMeter("##meter", out, numFrames);
      ImGui::EndVertical();
    }
  };

┌─────────────────────────────────────────────────────────────────┐
│  FASE 5: COMPILATION                                            │
└─────────────────────────────────────────────────────────────────┘

  MyFilter.cpp + MyFilter.hpp
       ↓
  ┌──────────────┐
  │ C++ Compiler │  g++ / clang++ / MSVC
  └──────────────┘  (with -O3 optimization)
       ↓
  MyFilter.o (object file)
       ↓
  ┌──────────────┐
  │ Linker       │  Link with MicroSuono engine
  └──────────────┘
       ↓
  Shared Library (.so / .dll / .dylib)
  
  OR (for standalone):
  ┌──────────────┐
  │ Static Link  │  Link into main executable
  └──────────────┘
       ↓
  MicroSuono executable (with MyFilter embedded)

┌─────────────────────────────────────────────────────────────────┐
│  FASE 6: LOADING & EXECUTION                                    │
└─────────────────────────────────────────────────────────────────┘

  Shared Library
       ↓
  ┌──────────────┐
  │ Dynamic Load │  dlopen() / LoadLibrary()
  └──────────────┘
       ↓
  Node Factory registered in GraphManager
       ↓
  User creates node in graph
       ↓
  Real-time audio processing (native C++ speed!)
```

---

## Dettaglio: Code Generation

### Input DSL
```dsl
process(sample) {
  out = in * gain;
}
```

### Generated C++
```cpp
void process(int numFrames) override {
  const float* in_ptr = getAudioInput(0);
  float* out_ptr = getAudioOutput(0);
  
  for (int i = 0; i < numFrames; ++i) {
    out_ptr[i] = in_ptr[i] * param_gain;
  }
}
```

### Compiled Assembly (x86_64, -O3)
```asm
; Vectorized loop (SIMD)
loop:
  vmovups ymm0, [rsi + rax*4]      ; Load 8 floats from in
  vmulps  ymm0, ymm0, ymm1         ; Multiply by gain (broadcasted)
  vmovups [rdi + rax*4], ymm0      ; Store 8 floats to out
  add     rax, 8
  cmp     rax, rcx
  jl      loop
```

**Risultato: Performance identica a C++ scritto a mano!**

---

## Performance Considerations

### Perché è Veloce Come C++?

1. **Compilazione Nativa**: DSL → C++ → Assembly ottimizzato
2. **Zero Overhead Runtime**: Nessun interprete, nessuna VM
3. **Ottimizzazioni Compiler**: Sfrutta g++/clang con -O3
4. **SIMD Auto-vectorization**: Loop semplici → istruzioni vettoriali
5. **Inline Functions**: Funzioni piccole integrate direttamente
6. **Static Typing**: Nessun controllo di tipo a runtime

### Benchmark Atteso

| Implementazione | Throughput (MSamples/s) | Overhead |
|-----------------|-------------------------|----------|
| C++ ottimizzato | 1000 | 0% (baseline) |
| DSL compilato   | 1000 | 0% |
| Python (NumPy)  | 100  | 90% |
| JavaScript      | 50   | 95% |
| Lua (interpreted) | 10 | 99% |

**Il DSL compilato ha le stesse performance del C++ equivalente.**

---

## Hot Reload (Sviluppo Rapido)

Durante lo sviluppo, l'utente può modificare il DSL e ricaricare il nodo **senza riavviare l'audio**:

```
User edits MyFilter.dsl
     ↓
File watcher detects change
     ↓
Recompile in background:
  DSL → C++ → .so (shared library)
     ↓
Unload old MyFilter.so
     ↓
Load new MyFilter.so
     ↓
Replace node instances in graph
     ↓
Audio continues with new code!
```

**Tempo di ricompilazione: ~1-2 secondi** (grazie a ccache e compilazione incrementale)

---

## Export VST3

Quando l'utente esporta il progetto come VST3:

```
MicroSuono Project (.msproj)
     ↓
┌────────────────────────┐
│  VST3 Export Generator │
└────────────────────────┘
     ↓
Generates:
  • VST3 plugin boilerplate (factory, controller, processor)
  • Embeds generated C++ code for all nodes
  • Maps GUI widgets to VST3 parameters
  • Maps process() to VST3 processBlock()
     ↓
CMake project for VST3
     ↓
User clicks "Build VST3"
     ↓
Compiles to MyPlugin.vst3
     ↓
Ready to use in DAW!
```

### Mapping DSL → VST3

| DSL Concept | VST3 Equivalent |
|-------------|-----------------|
| `param` | VST3 Parameter |
| `audio in/out` | VST3 Audio Bus |
| `event in/out` | VST3 Event Bus |
| `process()` | `IAudioProcessor::process()` |
| `onControl()` | Parameter smoothing/update |
| GUI widgets | VSTGUI controls or ImGui embedded |

---

## Strumenti di Sviluppo Necessari

### Parser & Lexer
- **Opzione 1**: Flex + Bison (generatori tradizionali)
- **Opzione 2**: Parser Combinators in C++ (più controllo)
- **Opzione 3**: ANTLR4 (potente, forse overkill)

**Raccomandazione**: **Parser combinators custom** (massima flessibilità, error reporting migliore)

### Type Checker
- Custom semantic analyzer
- Symbol table con scope nesting
- Type inference (opzionale, per semplificare sintassi)

### Code Generator
- Template-based C++ code generation
- Usa libreria come {fmt} o std::format per templating
- Genera codice leggibile e debuggabile

### Build System
- CMake per generare progetti
- ccache per compilazione rapida
- Ninja per build paralleli veloci

---

## Grammatica Formale (EBNF)

```ebnf
(* Node Definition *)
node_decl = "node" IDENTIFIER "{" node_body "}" ;

node_body = { port_decl | param_decl | state_decl | gui_decl | callback_decl } ;

(* Ports *)
port_decl = port_type IDENTIFIER { "," IDENTIFIER } ";" ;
port_type = "audio" | "control" | "event" ;

(* Parameters *)
param_decl = "param" IDENTIFIER "=" literal "{" param_options "}" ";" ;
param_options = param_option { "," param_option } ;
param_option = IDENTIFIER ":" literal ;

(* State *)
state_decl = "state" "{" { var_decl } "}" ";" ;
var_decl = type IDENTIFIER "=" literal ";" ;
type = "float" | "int" | "bool" ;

(* GUI *)
gui_decl = "gui" "{" { gui_statement } "}" ";" ;
gui_statement = gui_layout | gui_widget ;
gui_layout = "layout" ":" IDENTIFIER ";" ;
gui_widget = IDENTIFIER "(" argument_list ")" ";" ;

(* Callbacks *)
callback_decl = callback_name "(" [ parameter_list ] ")" "{" { statement } "}" ;
callback_name = "init" | "prepare" | "onControl" | "onEvent" | "process" | "onChange" ;

(* Statements *)
statement = assignment | if_statement | for_loop | emit_statement | expression ";" ;

assignment = IDENTIFIER "=" expression ";" ;
if_statement = "if" "(" expression ")" "{" { statement } "}" [ "else" "{" { statement } "}" ] ;
for_loop = "for" IDENTIFIER "in" range "{" { statement } "}" ;
emit_statement = "emit" IDENTIFIER "(" argument_list ")" ";" ;

(* Expressions *)
expression = binary_expr | unary_expr | call_expr | literal | IDENTIFIER ;
binary_expr = expression BINARY_OP expression ;
unary_expr = UNARY_OP expression ;
call_expr = IDENTIFIER "(" [ argument_list ] ")" ;

literal = NUMBER | STRING | "true" | "false" ;
```

---

## Esempi Avanzati

### 1. Wavetable Oscillator con GUI complessa

```dsl
node WavetableOsc {
  audio out;
  
  param frequency = 440.0 {
    min: 20.0, max: 20000.0, scale: "log", unit: "Hz"
  };
  
  param wavetable = "sine" {
    options: ["sine", "saw", "square", "triangle", "custom"]
  };
  
  state {
    float phase = 0.0;
    float table[2048];
  };
  
  gui {
    layout: vertical;
    
    waveform_display(table, size: large);
    dropdown(wavetable, label: "Waveform");
    slider(frequency, label: "Frequency", orientation: horizontal);
    phase_display(phase);
  };
  
  init() {
    // Fill wavetable
    for (i in 0..2047) {
      table[i] = sin(i * TWO_PI / 2048.0);
    }
  }
  
  onChange(wavetable) {
    // Regenerate wavetable when selection changes
    if (wavetable == "sine") {
      for (i in 0..2047) {
        table[i] = sin(i * TWO_PI / 2048.0);
      }
    } else if (wavetable == "saw") {
      for (i in 0..2047) {
        table[i] = 2.0 * (i / 2048.0) - 1.0;
      }
    }
    // ... other waveforms
  }
  
  process(sample) {
    // Linear interpolation
    float index_float = phase * 2048.0;
    int index = int(index_float);
    float frac = index_float - index;
    
    float v1 = table[index];
    float v2 = table[(index + 1) % 2048];
    
    out = lerp(v1, v2, frac);
    
    phase += frequency / sampleRate;
    if (phase >= 1.0) phase -= 1.0;
  }
}
```

### 2. Delay Line con Feedback

```dsl
node Delay {
  audio in, out;
  control feedback_mod;  // External modulation
  
  param delay_time = 0.5 {
    min: 0.001, max: 2.0, unit: "s"
  };
  
  param feedback = 0.5 {
    min: 0.0, max: 0.95
  };
  
  param mix = 0.5 {
    min: 0.0, max: 1.0
  };
  
  state {
    float buffer[88200];  // 2 seconds @ 44100 Hz
    int write_pos = 0;
    int delay_samples = 22050;
  };
  
  gui {
    layout: vertical;
    knob(delay_time, label: "Time", size: large);
    knob(feedback, label: "Feedback");
    knob(mix, label: "Mix");
    scope(out, style: "waveform");
  };
  
  prepare() {
    delay_samples = int(delay_time * sampleRate);
    delay_samples = clamp(delay_samples, 1, 88199);
  }
  
  onControl() {
    float fb = feedback;
    if (feedback_mod.isConnected()) {
      fb = clamp(feedback_mod.value, 0.0, 0.95);
    }
  }
  
  process(sample) {
    int read_pos = (write_pos - delay_samples + 88200) % 88200;
    float delayed = buffer[read_pos];
    
    buffer[write_pos] = in + delayed * feedback;
    write_pos = (write_pos + 1) % 88200;
    
    out = in * (1.0 - mix) + delayed * mix;
  }
}
```

---

## Timeline di Implementazione

### Fase 1: Parser & Type Checker (2-3 settimane)
- Grammatica formale
- Lexer + Parser
- AST construction
- Basic type checking

### Fase 2: Code Generator (2 settimane)
- C++ code generation
- Template system
- Build integration

### Fase 3: Compilation Pipeline (1 settimana)
- CMake integration
- Hot reload system
- Error reporting

### Fase 4: GUI Generation (2 settimane)
- ImGui widget mapping
- Layout engine
- Parameter binding

### Fase 5: VST3 Export (2-3 settimane)
- VST3 boilerplate generation
- Parameter mapping
- GUI embedding

**Totale: ~10-12 settimane per MVP completo**

---

## Conclusione

Il DSL di MicroSuono:

✅ **Semplice da scrivere** (niente header, puntatori, memoria)  
✅ **Veloce come C++** (compilazione nativa, zero overhead)  
✅ **Type-safe** (errori a compile-time)  
✅ **GUI integrata** (dichiarativa e potente)  
✅ **VST3-ready** (export automatico)  
✅ **Hot reload** (sviluppo rapido)  

**È la soluzione perfetta per il tuo progetto!** 🚀
