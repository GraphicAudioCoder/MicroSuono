#include <MicroSuono/MicroScript/Token.hpp>
#include <sstream>
#include <unordered_map>

namespace ms {
namespace micros {

static const std::unordered_map<TokenType, std::string> tokenNames = {
    // Keywords
    {TokenType::NODE, "NODE"},
    {TokenType::AUDIO, "AUDIO"},
    {TokenType::CONTROL, "CONTROL"},
    {TokenType::EVENT, "EVENT"},
    {TokenType::BUFFER, "BUFFER"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::FOR, "FOR"},
    {TokenType::IN, "IN"},
    {TokenType::MATCH, "MATCH"},

    // Decorators
    {TokenType::AT_PROCESS, "AT_PROCESS"},
    {TokenType::AT_PREPARE, "AT_PREPARE"},
    {TokenType::AT_CONTROL, "AT_CONTROL"},
    {TokenType::AT_EVENT, "AT_EVENT"},
    {TokenType::AT_CHANGE, "AT_CHANGE"},
    {TokenType::AT_MIN, "AT_MIN"},
    {TokenType::AT_MAX, "AT_MAX"},
    {TokenType::AT_SMOOTH, "AT_SMOOTH"},
    {TokenType::AT_UNIT, "AT_UNIT"},
    {TokenType::AT_DEFAULT, "AT_DEFAULT"},
    {TokenType::AT_MAX_SIZE, "AT_MAX_SIZE"},
    {TokenType::AT_OPTIONS, "AT_OPTIONS"},

    // Operators
    {TokenType::ARROW_LEFT, "ARROW_LEFT"},
    {TokenType::ARROW_RIGHT, "ARROW_RIGHT"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::STAR, "STAR"},
    {TokenType::SLASH, "SLASH"},
    {TokenType::PERCENT, "PERCENT"},
    {TokenType::EQUALS, "EQUALS"},
    {TokenType::EQUALS_EQUALS, "EQUALS_EQUALS"},
    {TokenType::NOT_EQUALS, "NOT_EQUALS"},
    {TokenType::LESS, "LESS"},
    {TokenType::LESS_EQUALS, "LESS_EQUALS"},
    {TokenType::GREATER, "GREATER"},
    {TokenType::GREATER_EQUALS, "GREATER_EQUALS"},
    {TokenType::AND, "AND"},
    {TokenType::OR, "OR"},
    {TokenType::NOT, "NOT"},

    // Delimiters
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RBRACE, "RBRACE"},
    {TokenType::LBRACKET, "LBRACKET"},
    {TokenType::RBRACKET, "RBRACKET"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::DOT, "DOT"},

    // Literals
    {TokenType::NUMBER, "NUMBER"},
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::STRING_LITERAL, "STRING_LITERAL"},
    {TokenType::TRUE_KW, "TRUE_KW"},
    {TokenType::FALSE_KW, "FALSE_KW"},

    // Special
    {TokenType::NEWLINE, "NEWLINE"},
    {TokenType::END_OF_FILE, "END_OF_FILE"}};

std::string Token::toString() const {
  std::ostringstream oss;
  auto it = tokenNames.find(type);
  std::string typeName = (it != tokenNames.end()) ? it->second : "UNKNOWN";

  oss << typeName << "(\"" << lexeme << "\") at " << line << ":" << column;
  return oss.str();
}

} // namespace micros
} // namespace ms
