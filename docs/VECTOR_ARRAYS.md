# MicroScript Vector & Array Support

## ✅ Implementato (Lexer-ready)

### 1. Dichiarazioni Vettoriali
```micros
// Input/output vettoriali
-> inputs[8] audio
-> gains[4] control
<- outputs[2] audio

// Variabili interne vettoriali
envelope[8] = [0.0; 8]           // Repeat syntax: [value; count]
coefficients[4] = [1.0, 0.5, 0.25, 0.125]  // Explicit values
```

### 2. Array 2D (Matrici)
```micros
// Matrix declaration
buffer[4][1024] = [[0.0; 1024]; 4]  // 4 bands × 1024 samples

// Access
buffer[band][position] = value
```

### 3. Operazioni su Array
```micros
// Indexing
value = array[i]
array[i] = new_value

// Range loops
for (i in 0..8) {
  process(array[i])
}

// Modulo indexing
pos = (pos + 1) % array.size()
```

### 4. Compound Assignments
```micros
array[i] += value
array[i] *= gain
array[i] -= offset
array[i] /= divisor
```

### 5. Operatori Supportati
- Aritmetici: `+`, `-`, `*`, `/`, `%`
- Comparazione: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Logici: `&&`, `||`, `!`
- Assignment: `=`, `+=`, `-=`, `*=`, `/=`
- Range: `..` (es. `0..10`)

## 🎯 Cosa Manca (Parser/Codegen)

### 1. Vector Operations (SIMD-style)
```micros
// Element-wise operations (TODO: Parser)
result = vec_a * vec_b           // Element-wise multiply
result = vec_a + scalar          // Broadcast scalar
result = abs(vec_a)              // Element-wise function
result = clamp(vec_a, 0.0, 1.0)  // Element-wise clamp
```

### 2. Funzioni Vettoriali Built-in
```micros
// Math functions (TODO: Implementare)
result = abs(vector)
result = sqrt(vector)
result = sin(vector)
result = pow(vector, exponent)

// Utility functions
sum = reduce_add(vector)         // Somma tutti gli elementi
max_val = reduce_max(vector)     // Massimo
min_val = reduce_min(vector)     // Minimo
avg = mean(vector)               // Media

// Interpolation
result = lerp(a, b, t)           // Linear interpolation
result = smoothstep(edge0, edge1, x)
```

### 3. Array Methods/Properties
```micros
len = array.size()               // Lunghezza
array.fill(0.0)                  // Riempi con valore
array.copy(source)               // Copia da altro array
```

## 📋 Raccomandazioni

### Feature Priority

#### 🔴 HIGH PRIORITY (Fondamentali)
1. **Variabili interne vettoriali** - Per state persistente
   - Envelope followers
   - Peak holders  
   - Circular buffers
   
2. **Array 2D (matrici)** - Per multi-band, convolution, ecc
   - Delay lines per band
   - Filter banks
   - Spectral processing

3. **Basic vector math** - Operazioni element-wise
   - `result = a * b` (vettori stessa dimensione)
   - `result = a + scalar` (broadcast)
   - `result = abs(a)`, `clamp()`, ecc.

#### 🟡 MEDIUM PRIORITY (Utili)
4. **Reduction operations** - Riduzione a scalare
   - `sum()`, `max()`, `min()`, `mean()`
   - Utile per metering, analysis

5. **Array utilities** - Helper methods
   - `.size()`, `.fill()`, `.clear()`
   - Circular buffer support: `(i + 1) % size`

#### 🟢 LOW PRIORITY (Nice-to-have)
6. **Advanced SIMD** - Operazioni complesse
   - FFT/IFFT per spectral processing
   - Convolution
   - IIR/FIR filter banks

7. **Dynamic sizing** - Array runtime sizing
   - Attualmente size è compile-time constant
   - Dynamic potrebbe essere complicato per DSP real-time

### Syntax Recommendations

#### ✅ GOOD - Chiara e consistente
```micros
// Vector declaration with initialization
envelope[8] = [0.0; 8]              // Repeat syntax
gains[4] = [1.0, 0.8, 0.6, 0.4]    // Explicit values

// 2D arrays
buffers[4][1024] = [[0.0; 1024]; 4]

// Access
value = array[index]
matrix[row][col] = value
```

#### ❌ AVOID - Ambiguo o complesso
```micros
// NO: Dynamic sizing (problematico per real-time)
array = new Array(size)

// NO: Slice syntax troppo complesso
subset = array[0:4]  // Troppo Pythonic

// NO: List comprehensions (troppo funzionale)
result = [x * 2 for x in array]
```

## 🔧 Implementazione Suggerita

### Step 1: Parser Support (Prossimo)
- Riconoscere dichiarazioni array: `name[size] = initializer`
- Riconoscere indexing: `array[expr]`
- Riconoscere 2D: `matrix[expr][expr]`

### Step 2: Type System
- Distinguere scalar vs vector type
- Type checking: `vector[8] = scalar` → error
- Automatic broadcast per alcuni casi

### Step 3: Codegen (C++)
```cpp
// MicroScript:
envelope[8] = [0.0; 8]
processed = signal * gains

// Generated C++:
std::array<float, 8> envelope{0.0f};
for (int i = 0; i < 8; ++i) {
  processed[i] = signal[i] * gains[i];
}
```

### Step 4: Optimization
- Detect loop vectorization opportunities
- Use SIMD intrinsics quando possibile
- Unroll small fixed-size loops

## 📝 Examples Coverage

| Feature | Gain | Sine | Delay | Mixer | VectorOps | Matrix |
|---------|------|------|-------|-------|-----------|--------|
| Scalar I/O | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Vector I/O | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ |
| Vector internal | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |
| 2D arrays | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| For loops | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Vector math | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |

## 🎓 Conclusione

Il **Lexer è completo** e supporta tutta la sintassi necessaria per:
- ✅ Array 1D e 2D
- ✅ Array literals con repeat syntax `[value; count]`  
- ✅ Indexing e accesso multi-dimensionale
- ✅ Range operator `..`
- ✅ Tutti gli operatori necessari

**Next Steps:**
1. **Parser** - Costruire AST per array operations
2. **Type Checker** - Verificare dimensioni e tipi
3. **Codegen** - Generare C++ efficiente
4. **Built-in functions** - abs(), clamp(), reduce_*, etc.

Il design è **pulito, leggibile e adatto al DSP real-time** 🎵
