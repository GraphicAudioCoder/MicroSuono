#include <MicroSuono/MicroScript/Lexer.hpp>
#include <cctype>
#include <iostream>
#include <unordered_map>
#include <variant>

namespace ms {
namespace micros {

// Mappa delle keywords
static const std::unordered_map<std::string, TokenType> keywords = {
    {"node", TokenType::NODE},
    {"audio", TokenType::AUDIO},
    {"control", TokenType::CONTROL},
    {"event", TokenType::EVENT},
    {"buffer", TokenType::BUFFER},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"match", TokenType::MATCH},
    {"true", TokenType::TRUE_KW},
    {"false", TokenType::FALSE_KW}
};

Lexer::Lexer(const std::string& source) 
    : source_(source), current_(0), start_(0), line_(1), column_(1) {}

bool Lexer::isAtEnd() const {
  return current_ >= source_.length();
}

char Lexer::advance() {
  column_++;
  return source_[current_++];
}

char Lexer::peek() const {
  if (isAtEnd()) return '\0';
  return source_[current_];
}

char Lexer::peekNext() const {
  if (current_ + 1 >= source_.length()) return '\0';
  return source_[current_ + 1];
}

bool Lexer::match(char expected) {
  if (isAtEnd()) return false;
  if (source_[current_] != expected) return false;
  current_++;
  column_++;
  return true;
}

void Lexer::skipWhitespace() {
  while (!isAtEnd()) {
    char c = peek();
    if (c == ' ' || c == '\r' || c == '\t') {
      advance();
    } else if (c == '\n') {
      advance();
      line_++;
      column_ = 1;
    } else {
      break;
    }
  }
}

void Lexer::skipComment() {
  // Commento single-line //
  // Salta fino alla fine della riga
  while (!isAtEnd() && peek() != '\n') {
    advance();
  }
}

void Lexer::addToken(TokenType type) {
  std::string text = source_.substr(start_, current_ - start_);
  int tokenColumn = column_ - static_cast<int>(text.length());
  tokens_.emplace_back(type, text, line_, tokenColumn);
}

void Lexer::addToken(TokenType type, std::variant<float, int, bool, std::string> value) {
  std::string text = source_.substr(start_, current_ - start_);
  int tokenColumn = column_ - static_cast<int>(text.length());
  tokens_.emplace_back(type, text, value, line_, tokenColumn);
}

void Lexer::scanNumber() {
  while (std::isdigit(peek())) {
    advance();
  }
  
  // Cerchiamo un punto decimale
  if (peek() == '.' && std::isdigit(peekNext())) {
    advance(); // consuma il '.'
    while (std::isdigit(peek())) {
      advance();
    }
    
    // È un float
    std::string numStr = source_.substr(start_, current_ - start_);
    float value = std::stof(numStr);
    addToken(TokenType::NUMBER, value);
  } else {
    // È un int
    std::string numStr = source_.substr(start_, current_ - start_);
    int value = std::stoi(numStr);
    addToken(TokenType::NUMBER, value);
  }
}

void Lexer::scanIdentifier() {
  while (std::isalnum(peek()) || peek() == '_') {
    advance();
  }
  
  std::string text = source_.substr(start_, current_ - start_);
  
  // Controlliamo se è una keyword
  auto it = keywords.find(text);
  if (it != keywords.end()) {
    addToken(it->second);
  } else {
    addToken(TokenType::IDENTIFIER, text);
  }
}

void Lexer::scanString() {
  while (!isAtEnd() && peek() != '"') {
    if (peek() == '\n') {
      line_++;
      column_ = 0;
    }
    advance();
  }
  
  if (isAtEnd()) {
    std::cerr << "Unterminated string at line " << line_ << std::endl;
    return;
  }
  
  // Consuma la " finale
  advance();
  
  // Estraiamo il valore della stringa (senza le virgolette)
  std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
  addToken(TokenType::STRING_LITERAL, value);
}

void Lexer::scanToken() {
  char c = advance();
  
  switch (c) {
    // Singoli caratteri
    case '(': addToken(TokenType::LPAREN); break;
    case ')': addToken(TokenType::RPAREN); break;
    case '{': addToken(TokenType::LBRACE); break;
    case '}': addToken(TokenType::RBRACE); break;
    case '[': addToken(TokenType::LBRACKET); break;
    case ']': addToken(TokenType::RBRACKET); break;
    case ',': addToken(TokenType::COMMA); break;
    case ';': addToken(TokenType::SEMICOLON); break;
    case '.': 
      // Check for .. (range operator)
      if (match('.')) {
        addToken(TokenType::DOT_DOT);
      } else {
        addToken(TokenType::DOT);
      }
      break;
    case '+': 
      addToken(match('=') ? TokenType::PLUS_EQUALS : TokenType::PLUS);
      break;
    case '*': 
      addToken(match('=') ? TokenType::STAR_EQUALS : TokenType::STAR);
      break;
    case '%': addToken(TokenType::PERCENT); break;
    
    // Operatori che possono essere doppi
    case '=':
      addToken(match('=') ? TokenType::EQUALS_EQUALS : TokenType::EQUALS);
      break;
    case '!':
      addToken(match('=') ? TokenType::NOT_EQUALS : TokenType::NOT);
      break;
    case '<':
      if (match('-')) {
        addToken(TokenType::ARROW_LEFT);
      } else if (match('=')) {
        addToken(TokenType::LESS_EQUALS);
      } else {
        addToken(TokenType::LESS);
      }
      break;
    case '>':
      addToken(match('=') ? TokenType::GREATER_EQUALS : TokenType::GREATER);
      break;
    case '-':
      if (match('>')) {
        addToken(TokenType::ARROW_RIGHT);
      } else if (match('=')) {
        addToken(TokenType::MINUS_EQUALS);
      } else {
        addToken(TokenType::MINUS);
      }
      break;
    case '&':
      if (match('&')) addToken(TokenType::AND);
      break;
    case '|':
      if (match('|')) addToken(TokenType::OR);
      break;
    
    // Slash o commento
    case '/':
      if (peek() == '/') {
        advance(); // Consuma il secondo '/'
        skipComment();
      } else if (match('=')) {
        addToken(TokenType::SLASH_EQUALS);
      } else {
        addToken(TokenType::SLASH);
      }
      break;
    
    // Decoratori @
    case '@': {
      start_ = current_; // Riparte da dopo @
      while (std::isalpha(peek()) || peek() == '_') {
        advance();
      }
      std::string decoratorName = source_.substr(start_, current_ - start_);
      
      if (decoratorName == "process") addToken(TokenType::AT_PROCESS);
      else if (decoratorName == "prepare") addToken(TokenType::AT_PREPARE);
      else if (decoratorName == "control") addToken(TokenType::AT_CONTROL);
      else if (decoratorName == "event") addToken(TokenType::AT_EVENT);
      else if (decoratorName == "change") addToken(TokenType::AT_CHANGE);
      else if (decoratorName == "min") addToken(TokenType::AT_MIN);
      else if (decoratorName == "max") addToken(TokenType::AT_MAX);
      else if (decoratorName == "smooth") addToken(TokenType::AT_SMOOTH);
      else if (decoratorName == "unit") addToken(TokenType::AT_UNIT);
      else if (decoratorName == "default") addToken(TokenType::AT_DEFAULT);
      else if (decoratorName == "max_size") addToken(TokenType::AT_MAX_SIZE);
      else if (decoratorName == "options") addToken(TokenType::AT_OPTIONS);
      else if (decoratorName == "fadein") addToken(TokenType::AT_FADEIN);
      else if (decoratorName == "fadeout") addToken(TokenType::AT_FADEOUT);
      else {
        std::cerr << "Unknown decorator: @" << decoratorName << " at line " << line_ << std::endl;
      }
      break;
    }
    
    // Stringhe
    case '"':
      scanString();
      break;
    
    default:
      if (std::isdigit(c)) {
        scanNumber();
      } else if (std::isalpha(c) || c == '_') {
        scanIdentifier();
      } else {
        std::cerr << "Unexpected character '" << c << "' at line " << line_ << std::endl;
      }
      break;
  }
}

std::vector<Token> Lexer::tokenize() {
  while (!isAtEnd()) {
    skipWhitespace();
    if (!isAtEnd()) {
      start_ = current_;  // Imposta start DOPO aver saltato whitespace
      scanToken();
    }
  }
  
  tokens_.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
  return tokens_;
}

} // namespace micros
} // namespace ms
