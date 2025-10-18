#pragma once
#include <MicroSuono/MicroScript/Token.hpp>
#include <string>
#include <variant>
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

  // Helper methods
  char advance();
  char peek() const;
  char peekNext() const;
  bool isAtEnd() const;
  bool match(char expected);
  void skipWhitespace();
  void skipComment();

  // Token generation
  void addToken(TokenType type);
  void addToken(TokenType type,
                std::variant<float, int, bool, std::string> value);

  // Scanning
  void scanToken();
  void scanNumber();
  void scanIdentifier();
  void scanString();

public:
  Lexer(const std::string &source);
  std::vector<Token> tokenize();
};

} // namespace micros
} // namespace ms
