#pragma once
#include <string>
#include <variant>

namespace ms {
namespace micros {

enum class TokenType {
  // Keywords
  NODE,
  AUDIO,
  CONTROL,
  EVENT,
  BUFFER,
  IF,
  ELSE,
  FOR,
  IN,
  MATCH,

  // Decorators
  AT_PROCESS,
  AT_PREPARE,
  AT_CONTROL,
  AT_EVENT,
  AT_CHANGE,
  AT_MIN,
  AT_MAX,
  AT_SMOOTH,
  AT_UNIT,
  AT_DEFAULT,
  AT_MAX_SIZE,
  AT_OPTIONS,

  // Operators
  ARROW_LEFT,  // <-
  ARROW_RIGHT, // ->
  PLUS,
  MINUS,
  STAR,
  SLASH,
  PERCENT,
  EQUALS,
  EQUALS_EQUALS,
  NOT_EQUALS,
  LESS,
  LESS_EQUALS,
  GREATER,
  GREATER_EQUALS,
  AND,
  OR,
  NOT,

  // Delimiters
  LPAREN,
  RPAREN, // ( )
  LBRACE,
  RBRACE, // { }
  LBRACKET,
  RBRACKET, // [ ]
  COMMA,
  DOT,

  // Literals
  NUMBER,
  IDENTIFIER,
  STRING_LITERAL,
  TRUE_KW,
  FALSE_KW,

  // Special
  COMMENT,
  NEWLINE,
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string lexeme;
  std::variant<float, int, bool, std::string> value;
  int line;
  int column;

  Token(TokenType t, std::string lex, int l, int c)
      : type(t), lexeme(lex), line(l), column(c) {}
  Token(TokenType t, std::string lex,
        std::variant<float, int, bool, std::string> val, int l, int c)
      : type(t), lexeme(lex), value(val), line(l), column(c) {}

  std::string toString() const;
};

} // namespace micros
} // namespace ms
