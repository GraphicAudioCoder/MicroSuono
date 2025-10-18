# Tutorial: Implementare il Lexer per MicroScript

## Indice
1. [Introduzione](#introduzione)
2. [Cos'è un Lexer](#cosè-un-lexer)
3. [Architettura del Lexer](#architettura-del-lexer)
4. [Step 1: Struttura Base](#step-1-struttura-base)
5. [Step 2: Gestione Caratteri](#step-2-gestione-caratteri)
6. [Step 3: Numeri](#step-3-numeri)
7. [Step 4: Identificatori e Keywords](#step-4-identificatori-e-keywords)
8. [Step 5: Operatori](#step-5-operatori)
9. [Step 6: Delimitatori](#step-6-delimitatori)
10. [Step 7: Commenti](#step-7-commenti)
11. [Step 8: String Literals](#step-8-string-literals)
12. [Step 9: Whitespace e Newlines](#step-9-whitespace-e-newlines)
13. [Step 10: Error Handling](#step-10-error-handling)
14. [Step 11: Testing](#step-11-testing)
15. [Step 12: Integrazione](#step-12-integrazione)

---

## Introduzione

Questo tutorial ti guiderà passo dopo passo nell'implementazione di un **Lexer (Analizzatore Lessicale)** per MicroScript. 

Il Lexer è il primo componente della pipeline di compilazione:
```
Source Code → LEXER → Tokens → Parser → AST → Code Generator → C++
```

**Obiettivo**: Trasformare testo sorgente MicroScript in una sequenza di `Token` definiti in `Token.hpp`.

---

## Cos'è un Lexer

Il **Lexer** (o Scanner) legge il codice sorgente carattere per carattere e lo suddivide in **token** (unità lessicali minime).

### Esempio

**Input (MicroScript):**
```microscript
node Gain {
  -> gain control = 1.0 @smooth
  <- out audio
}
```

**Output (Token Stream):**
```
[NODE, "node", line=1, col=1]
[IDENTIFIER, "Gain", line=1, col=6]
[LBRACE, "{", line=1, col=11]
[ARROW_RIGHT, "->", line=2, col=3]
[IDENTIFIER, "gain", line=2, col=6]
[CONTROL, "control", line=2, col=11]
[EQUALS, "=", line=2, col=19]
[NUMBER, "1.0", value=1.0, line=2, col=21]
[AT_SMOOTH, "@smooth", line=2, col=25]
[ARROW_LEFT, "<-", line=3, col=3]
[IDENTIFIER, "out", line=3, col=6]
[AUDIO, "audio", line=3, col=10]
[RBRACE, "}", line=4, col=1]
[END_OF_FILE, "", line=4, col=2]
```

---

## Architettura del Lexer

### File da Creare

```
include/MicroSuono/MicroScript/
  ├── Token.hpp      (✅ già esistente)
  └── Lexer.hpp      (da creare)

src/MicroScript/
  ├── Token.cpp      (✅ già esistente)
  └── Lexer.cpp      (da creare)
```

### Struttura della Classe Lexer

```cpp
class Lexer {
private:
  std::string source_;        // Codice sorgente
  size_t current_;            // Posizione corrente
  size_t start_;              // Inizio del token corrente
  int line_;                  // Riga corrente
  int column_;                // Colonna corrente
  std::vector<Token> tokens_; // Lista di token generati

  // Helper methods
  char advance();
  char peek();
  char peekNext();
  bool isAtEnd();
  bool match(char expected);
  
  // Token generation
  void addToken(TokenType type);
  void addToken(TokenType type, std::variant<...> value);
  
  // Scanners
  void scanNumber();
  void scanIdentifier();
  void scanString();
  void scanComment();

public:
  Lexer(const std::string& source);
  std::vector<Token> tokenize();
};
```

---

## Step 1: Struttura Base

### File: `include/MicroSuono/MicroScript/Lexer.hpp`

```cpp
#pragma once
#include "Token.hpp"
#include <string>
#include <vector>

namespace ms {
namespace micros {

class Lexer {
private:
  std::string source_;
  size_t current_ = 0;
  size_t start_ = 0;
  int line_ = 1;
  int column_ = 1;
  std::vector<Token> tokens_;

  // Helper: avanza di un carattere
  char advance();
  
  // Helper: guarda il carattere corrente senza avanzare
  char peek() const;
  
  // Helper: guarda il prossimo carattere
  char peekNext() const;
  
  // Helper: siamo alla fine del sorgente?
  bool isAtEnd() const;
  
  // Helper: match condizionale
  bool match(char expected);
  
  // Scansiona un singolo token
  void scanToken();
  
  // Aggiungi token alla lista
  void addToken(TokenType type);
  void addToken(TokenType type, std::variant<float, int, bool, std::string> value);

public:
  explicit Lexer(const std::string& source);
  
  // Tokenizza tutto il sorgente
  std::vector<Token> tokenize();
};

} // namespace micros
} // namespace ms
```

### File: `src/MicroScript/Lexer.cpp`

```cpp
#include "MicroSuono/MicroScript/Lexer.hpp"
#include <stdexcept>

namespace ms {
namespace micros {

Lexer::Lexer(const std::string& source) 
  : source_(source) {}

std::vector<Token> Lexer::tokenize() {
  while (!isAtEnd()) {
    start_ = current_;
    scanToken();
  }
  
  // Aggiungi END_OF_FILE
  tokens_.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
  return tokens_;
}

bool Lexer::isAtEnd() const {
  return current_ >= source_.length();
}

char Lexer::peek() const {
  if (isAtEnd()) return '\0';
  return source_[current_];
}

char Lexer::peekNext() const {
  if (current_ + 1 >= source_.length()) return '\0';
  return source_[current_ + 1];
}

char Lexer::advance() {
  column_++;
  return source_[current_++];
}

bool Lexer::match(char expected) {
  if (isAtEnd()) return false;
  if (source_[current_] != expected) return false;
  
  current_++;
  column_++;
  return true;
}

void Lexer::addToken(TokenType type) {
  std::string lexeme = source_.substr(start_, current_ - start_);
  tokens_.emplace_back(type, lexeme, line_, column_ - lexeme.length());
}

void Lexer::addToken(TokenType type, std::variant<float, int, bool, std::string> value) {
  std::string lexeme = source_.substr(start_, current_ - start_);
  tokens_.emplace_back(type, lexeme, value, line_, column_ - lexeme.length());
}

void Lexer::scanToken() {
  char c = advance();
  
  // TODO: implementare lo switch per ogni carattere
  // Per ora, solo un placeholder
  switch (c) {
    case ' ':
    case '\r':
    case '\t':
      // Ignora whitespace
      break;
    
    case '\n':
      line_++;
      column_ = 1;
      addToken(TokenType::NEWLINE);
      break;
    
    default:
      throw std::runtime_error("Unexpected character: " + std::string(1, c));
  }
}

} // namespace micros
} // namespace ms
```

**✅ Checkpoint 1**: Compila e testa che la struttura base funzioni.

---

## Step 2: Gestione Caratteri

Implementiamo helper per classificare i caratteri:

```cpp
// In Lexer.hpp (private):
static bool isDigit(char c);
static bool isAlpha(char c);
static bool isAlphaNumeric(char c);

// In Lexer.cpp:
bool Lexer::isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
  return isAlpha(c) || isDigit(c);
}
```

---

## Step 3: Numeri

### Riconoscere Numeri (interi e float)

```cpp
// In Lexer.hpp (private):
void scanNumber();

// In Lexer.cpp:
void Lexer::scanNumber() {
  // Consuma cifre intere
  while (isDigit(peek())) {
    advance();
  }
  
  // Controlla se c'è un punto decimale
  bool isFloat = false;
  if (peek() == '.' && isDigit(peekNext())) {
    isFloat = true;
    advance(); // Consuma il '.'
    
    // Consuma cifre decimali
    while (isDigit(peek())) {
      advance();
    }
  }
  
  // Converti in numero
  std::string numberStr = source_.substr(start_, current_ - start_);
  
  if (isFloat) {
    float value = std::stof(numberStr);
    addToken(TokenType::NUMBER, value);
  } else {
    int value = std::stoi(numberStr);
    addToken(TokenType::NUMBER, value);
  }
}
```

### Aggiorna `scanToken()`:

```cpp
void Lexer::scanToken() {
  char c = advance();
  
  switch (c) {
    // ... whitespace ...
    
    default:
      if (isDigit(c)) {
        scanNumber();
      } else {
        throw std::runtime_error("Unexpected character: " + std::string(1, c));
      }
  }
}
```

**✅ Checkpoint 2**: Testa con `"123"`, `"3.14"`, `"0.5"`.

---

## Step 4: Identificatori e Keywords

### Riconoscere Identificatori

```cpp
// In Lexer.hpp (private):
void scanIdentifier();
TokenType identifierType(const std::string& text);

// In Lexer.cpp:
void Lexer::scanIdentifier() {
  while (isAlphaNumeric(peek())) {
    advance();
  }
  
  std::string text = source_.substr(start_, current_ - start_);
  TokenType type = identifierType(text);
  addToken(type);
}

TokenType Lexer::identifierType(const std::string& text) {
  // Keywords
  if (text == "node") return TokenType::NODE;
  if (text == "audio") return TokenType::AUDIO;
  if (text == "control") return TokenType::CONTROL;
  if (text == "event") return TokenType::EVENT;
  if (text == "buffer") return TokenType::BUFFER;
  if (text == "if") return TokenType::IF;
  if (text == "else") return TokenType::ELSE;
  if (text == "for") return TokenType::FOR;
  if (text == "in") return TokenType::IN;
  if (text == "match") return TokenType::MATCH;
  if (text == "true") return TokenType::TRUE_KW;
  if (text == "false") return TokenType::FALSE_KW;
  
  // Non è una keyword, è un identificatore
  return TokenType::IDENTIFIER;
}
```

### Aggiorna `scanToken()`:

```cpp
default:
  if (isDigit(c)) {
    scanNumber();
  } else if (isAlpha(c)) {
    scanIdentifier();
  } else {
    throw std::runtime_error("Unexpected character: " + std::string(1, c));
  }
```

**✅ Checkpoint 3**: Testa con `"node"`, `"Gain"`, `"frequency"`, `"true"`.

---

## Step 5: Operatori

### Riconoscere Operatori Singoli e Doppi

```cpp
void Lexer::scanToken() {
  char c = advance();
  
  switch (c) {
    // Single-character tokens
    case '(': addToken(TokenType::LPAREN); break;
    case ')': addToken(TokenType::RPAREN); break;
    case '{': addToken(TokenType::LBRACE); break;
    case '}': addToken(TokenType::RBRACE); break;
    case '[': addToken(TokenType::LBRACKET); break;
    case ']': addToken(TokenType::RBRACKET); break;
    case ',': addToken(TokenType::COMMA); break;
    case '.': addToken(TokenType::DOT); break;
    case '+': addToken(TokenType::PLUS); break;
    case '*': addToken(TokenType::STAR); break;
    case '%': addToken(TokenType::PERCENT); break;
    
    // Operatori che possono essere doppi
    case '-':
      if (match('>')) {
        addToken(TokenType::ARROW_RIGHT); // ->
      } else {
        addToken(TokenType::MINUS);
      }
      break;
    
    case '<':
      if (match('-')) {
        addToken(TokenType::ARROW_LEFT); // <-
      } else if (match('=')) {
        addToken(TokenType::LESS_EQUALS); // <=
      } else {
        addToken(TokenType::LESS); // <
      }
      break;
    
    case '>':
      if (match('=')) {
        addToken(TokenType::GREATER_EQUALS); // >=
      } else {
        addToken(TokenType::GREATER); // >
      }
      break;
    
    case '=':
      if (match('=')) {
        addToken(TokenType::EQUALS_EQUALS); // ==
      } else {
        addToken(TokenType::EQUALS); // =
      }
      break;
    
    case '!':
      if (match('=')) {
        addToken(TokenType::NOT_EQUALS); // !=
      } else {
        addToken(TokenType::NOT); // !
      }
      break;
    
    case '&':
      if (match('&')) {
        addToken(TokenType::AND); // &&
      } else {
        throw std::runtime_error("Expected '&' after '&'");
      }
      break;
    
    case '|':
      if (match('|')) {
        addToken(TokenType::OR); // ||
      } else {
        throw std::runtime_error("Expected '|' after '|'");
      }
      break;
    
    // ... rest of cases ...
  }
}
```

**✅ Checkpoint 4**: Testa con `"->"`, `"<-"`, `"=="`, `"!="`, `">="`, `"<="`.

---

## Step 6: Delimitatori

I delimitatori sono già gestiti nello Step 5 (parentesi, graffe, virgole, ecc.).

**Test**: `"{ ( ) [ ] , . }"`

---

## Step 7: Commenti

### Riconoscere `//` Comments

```cpp
case '/':
  if (match('/')) {
    // Commento: consuma fino a fine riga
    scanComment();
  } else {
    addToken(TokenType::SLASH);
  }
  break;

// In Lexer.cpp:
void Lexer::scanComment() {
  // Consuma fino a '\n' o fine file
  while (peek() != '\n' && !isAtEnd()) {
    advance();
  }
  
  // Aggiungi token COMMENT (opzionale, spesso i commenti vengono ignorati)
  std::string comment = source_.substr(start_, current_ - start_);
  addToken(TokenType::COMMENT);
}
```

**Alternativa**: Ignora i commenti completamente (non creare token).

```cpp
void Lexer::scanComment() {
  while (peek() != '\n' && !isAtEnd()) {
    advance();
  }
  // Non creare token, semplicemente salta il commento
}
```

**✅ Checkpoint 5**: Testa con `"// This is a comment\nnode Gain"`.

---

## Step 8: String Literals

### Riconoscere Stringhe tra Virgolette

```cpp
case '"':
  scanString();
  break;

// In Lexer.cpp:
void Lexer::scanString() {
  std::string value;
  
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      line_++;
      column_ = 0;
    }
    value += advance();
  }
  
  if (isAtEnd()) {
    throw std::runtime_error("Unterminated string");
  }
  
  // Consuma la " finale
  advance();
  
  addToken(TokenType::STRING_LITERAL, value);
}
```

**✅ Checkpoint 6**: Testa con `"\"Hello, World!\""`.

---

## Step 9: Whitespace e Newlines

```cpp
case ' ':
case '\r':
case '\t':
  // Ignora whitespace (non creare token)
  break;

case '\n':
  line_++;
  column_ = 1;
  // Opzionale: addToken(TokenType::NEWLINE);
  // Oppure ignora completamente
  break;
```

**Nota**: Alcuni parser hanno bisogno di `NEWLINE` token, altri no. Per MicroScript, probabilmente possiamo ignorarli.

---

## Step 10: Decoratori (`@`)

### Riconoscere `@process`, `@smooth`, ecc.

```cpp
case '@':
  scanDecorator();
  break;

// In Lexer.cpp:
void Lexer::scanDecorator() {
  // Consuma l'identificatore dopo '@'
  while (isAlphaNumeric(peek())) {
    advance();
  }
  
  std::string text = source_.substr(start_ + 1, current_ - start_ - 1); // Senza '@'
  TokenType type = decoratorType(text);
  addToken(type);
}

TokenType Lexer::decoratorType(const std::string& text) {
  if (text == "process") return TokenType::AT_PROCESS;
  if (text == "prepare") return TokenType::AT_PREPARE;
  if (text == "control") return TokenType::AT_CONTROL;
  if (text == "event") return TokenType::AT_EVENT;
  if (text == "change") return TokenType::AT_CHANGE;
  if (text == "min") return TokenType::AT_MIN;
  if (text == "max") return TokenType::AT_MAX;
  if (text == "smooth") return TokenType::AT_SMOOTH;
  if (text == "unit") return TokenType::AT_UNIT;
  if (text == "default") return TokenType::AT_DEFAULT;
  if (text == "max_size") return TokenType::AT_MAX_SIZE;
  if (text == "options") return TokenType::AT_OPTIONS;
  if (text == "fadein") return TokenType::AT_FADEIN;
  if (text == "fadeout") return TokenType::AT_FADEOUT;
  
  throw std::runtime_error("Unknown decorator: @" + text);
}
```

**✅ Checkpoint 7**: Testa con `"@process"`, `"@smooth"`, `"@min(0)"`.

---

## Step 11: Error Handling

### Migliorare i Messaggi di Errore

```cpp
void Lexer::reportError(const std::string& message) {
  throw std::runtime_error(
    "[Line " + std::to_string(line_) + 
    ", Col " + std::to_string(column_) + 
    "] " + message
  );
}

// Esempio di uso:
default:
  if (isDigit(c)) {
    scanNumber();
  } else if (isAlpha(c)) {
    scanIdentifier();
  } else {
    reportError("Unexpected character: '" + std::string(1, c) + "'");
  }
```

---

## Step 12: Testing

### Crea `tests/lexer_test.cpp`

```cpp
#include "MicroSuono/MicroScript/Lexer.hpp"
#include <iostream>
#include <cassert>

using namespace ms::micros;

void testBasicTokens() {
  std::string source = "node Gain { }";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  assert(tokens.size() == 5); // node, Gain, {, }, EOF
  assert(tokens[0].type == TokenType::NODE);
  assert(tokens[1].type == TokenType::IDENTIFIER);
  assert(tokens[1].lexeme == "Gain");
  assert(tokens[2].type == TokenType::LBRACE);
  assert(tokens[3].type == TokenType::RBRACE);
  assert(tokens[4].type == TokenType::END_OF_FILE);
  
  std::cout << "✅ testBasicTokens passed\n";
}

void testNumbers() {
  std::string source = "123 3.14 0.5";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  assert(tokens.size() == 4); // 3 numbers + EOF
  assert(tokens[0].type == TokenType::NUMBER);
  assert(tokens[1].type == TokenType::NUMBER);
  assert(tokens[2].type == TokenType::NUMBER);
  
  std::cout << "✅ testNumbers passed\n";
}

void testOperators() {
  std::string source = "-> <- == != >= <=";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  assert(tokens[0].type == TokenType::ARROW_RIGHT);
  assert(tokens[1].type == TokenType::ARROW_LEFT);
  assert(tokens[2].type == TokenType::EQUALS_EQUALS);
  assert(tokens[3].type == TokenType::NOT_EQUALS);
  assert(tokens[4].type == TokenType::GREATER_EQUALS);
  assert(tokens[5].type == TokenType::LESS_EQUALS);
  
  std::cout << "✅ testOperators passed\n";
}

void testDecorators() {
  std::string source = "@process @smooth @min @max @fadein";
  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  assert(tokens[0].type == TokenType::AT_PROCESS);
  assert(tokens[1].type == TokenType::AT_SMOOTH);
  assert(tokens[2].type == TokenType::AT_MIN);
  assert(tokens[3].type == TokenType::AT_MAX);
  assert(tokens[4].type == TokenType::AT_FADEIN);
  
  std::cout << "✅ testDecorators passed\n";
}

void testCompleteNode() {
  std::string source = R"(
    node Gain {
      -> gain control = 1.0 @smooth
      <- out audio
      
      @process {
        out = in * gain
      }
    }
  )";
  
  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  // Verifica che ci siano token aspettati
  bool hasNode = false;
  bool hasGain = false;
  bool hasSmooth = false;
  
  for (const auto& token : tokens) {
    if (token.type == TokenType::NODE) hasNode = true;
    if (token.lexeme == "Gain") hasGain = true;
    if (token.type == TokenType::AT_SMOOTH) hasSmooth = true;
  }
  
  assert(hasNode);
  assert(hasGain);
  assert(hasSmooth);
  
  std::cout << "✅ testCompleteNode passed\n";
}

int main() {
  try {
    testBasicTokens();
    testNumbers();
    testOperators();
    testDecorators();
    testCompleteNode();
    
    std::cout << "\n🎉 All tests passed!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "❌ Test failed: " << e.what() << "\n";
    return 1;
  }
}
```

### Aggiungi al CMakeLists.txt

```cmake
# Test del Lexer
add_executable(lexer_test tests/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE MicroSuonoLib)
```

**✅ Checkpoint 8**: Esegui `./build/lexer_test` e verifica che tutti i test passino.

---

## Step 13: Integrazione con Gain.micros

### Test Reale con File

```cpp
#include "MicroSuono/MicroScript/Lexer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

int main() {
  // Leggi Gain.micros
  std::ifstream file("examples/MicroScript/Gain.micros");
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  
  // Tokenizza
  ms::micros::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  
  // Stampa tutti i token
  for (const auto& token : tokens) {
    std::cout << token.toString() << "\n";
  }
  
  return 0;
}
```

**Output atteso:**
```
[NODE, "node", line=1, col=1]
[IDENTIFIER, "Gain", line=1, col=6]
[LBRACE, "{", line=1, col=11]
[ARROW_RIGHT, "->", line=2, col=3]
[IDENTIFIER, "in", line=2, col=6]
[AUDIO, "audio", line=2, col=9]
...
```

---

## Step 14: Ottimizzazioni e Raffinamenti

### 1. **Token Pooling** (Opzionale)
Per performance, puoi riutilizzare oggetti Token invece di crearli ogni volta.

### 2. **Error Recovery**
Invece di lanciare eccezioni, registra errori e continua a scansionare:

```cpp
struct LexerError {
  std::string message;
  int line;
  int column;
};

class Lexer {
private:
  std::vector<LexerError> errors_;
  
  void reportError(const std::string& message) {
    errors_.push_back({message, line_, column_});
    // Non lanciare eccezione, continua
  }
  
public:
  bool hasErrors() const { return !errors_.empty(); }
  const std::vector<LexerError>& getErrors() const { return errors_; }
};
```

### 3. **Supporto per Unicode** (Futuro)
Se vuoi supportare caratteri UTF-8 negli identificatori.

---

## Step 15: Checklist Finale

### ✅ Funzionalità Implementate

- [ ] Numeri (int e float)
- [ ] Identificatori
- [ ] Keywords (`node`, `audio`, `control`, ecc.)
- [ ] Operatori (`->`, `<-`, `==`, `!=`, ecc.)
- [ ] Delimitatori (`{`, `}`, `(`, `)`, ecc.)
- [ ] Decoratori (`@smooth`, `@process`, ecc.)
- [ ] Commenti (`//`)
- [ ] String literals (`"..."`)
- [ ] Whitespace e newline handling
- [ ] Error handling con line/column
- [ ] Token `END_OF_FILE`

### ✅ Test Completati

- [ ] Test numeri
- [ ] Test identificatori e keywords
- [ ] Test operatori
- [ ] Test decoratori
- [ ] Test commenti
- [ ] Test stringhe
- [ ] Test completo con Gain.micros
- [ ] Test error handling

---

## Prossimi Passi

Una volta completato il Lexer:

1. **Parser**: Trasforma token stream in AST (Abstract Syntax Tree)
2. **Semantic Analysis**: Verifica tipi, connessioni, ecc.
3. **Code Generator**: Genera C++ da AST
4. **Integration**: Integra con il sistema di build

---

## Risorse Utili

### Libri
- **"Crafting Interpreters"** di Robert Nystrom (disponibile gratis online)
- **"Compilers: Principles, Techniques, and Tools"** (Dragon Book)

### Online
- [Crafting Interpreters - Scanning](https://craftinginterpreters.com/scanning.html)
- [Lexical Analysis Tutorial](https://en.wikipedia.org/wiki/Lexical_analysis)

### Progetti Simili
- [Rust Compiler Lexer](https://github.com/rust-lang/rust/tree/master/compiler/rustc_lexer)
- [Python Tokenizer](https://github.com/python/cpython/blob/main/Parser/tokenizer.c)

---

## Esempio Completo: Lexer.hpp e Lexer.cpp

### `Lexer.hpp` (Completo)

```cpp
#pragma once
#include "Token.hpp"
#include <string>
#include <vector>

namespace ms {
namespace micros {

class Lexer {
private:
  std::string source_;
  size_t current_ = 0;
  size_t start_ = 0;
  int line_ = 1;
  int column_ = 1;
  std::vector<Token> tokens_;

  // Navigation
  char advance();
  char peek() const;
  char peekNext() const;
  bool isAtEnd() const;
  bool match(char expected);

  // Character classification
  static bool isDigit(char c);
  static bool isAlpha(char c);
  static bool isAlphaNumeric(char c);

  // Scanners
  void scanToken();
  void scanNumber();
  void scanIdentifier();
  void scanString();
  void scanComment();
  void scanDecorator();

  // Token creation
  void addToken(TokenType type);
  void addToken(TokenType type, std::variant<float, int, bool, std::string> value);

  // Helpers
  TokenType identifierType(const std::string& text);
  TokenType decoratorType(const std::string& text);
  void reportError(const std::string& message);

public:
  explicit Lexer(const std::string& source);
  std::vector<Token> tokenize();
};

} // namespace micros
} // namespace ms
```

### `Lexer.cpp` (Skeleton - da completare seguendo gli step)

```cpp
#include "MicroSuono/MicroScript/Lexer.hpp"
#include <stdexcept>
#include <cctype>

namespace ms {
namespace micros {

Lexer::Lexer(const std::string& source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
  while (!isAtEnd()) {
    start_ = current_;
    scanToken();
  }
  
  tokens_.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
  return tokens_;
}

// Implementa tutti i metodi seguendo gli step 1-10...

} // namespace micros
} // namespace ms
```

---

## Conclusione

Questo tutorial ti ha guidato attraverso tutti i passaggi necessari per implementare un Lexer completo per MicroScript. 

**Approccio consigliato:**
1. Implementa uno step alla volta
2. Testa dopo ogni step
3. Committa dopo ogni checkpoint ✅
4. Non passare allo step successivo finché il corrente non funziona

**Tempo stimato:** 1-2 giorni di lavoro concentrato.

Buona fortuna! 🚀
