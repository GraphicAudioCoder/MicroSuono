#include <MicroSuono/MicroScript/Lexer.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open file '" << path << "'" << std::endl;
    return "";
  }
  
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path-to-micros-file>" << std::endl;
    std::cerr << "Example: " << argv[0] << " examples/MicroScript/Gain.micros" << std::endl;
    return 1;
  }
  
  std::string filePath = argv[1];
  std::cout << "Reading file: " << filePath << std::endl;
  
  std::string source = readFile(filePath);
  if (source.empty()) {
    std::cerr << "Error: File is empty or could not be read" << std::endl;
    return 1;
  }
  
  std::cout << "\n=== Source Code ===" << std::endl;
  std::cout << source << std::endl;
  
  std::cout << "\n=== Tokenizing ===" << std::endl;
  ms::micros::Lexer lexer(source);
  std::vector<ms::micros::Token> tokens = lexer.tokenize();
  
  std::cout << "\n=== Tokens (" << tokens.size() << " total) ===" << std::endl;
  int count = 0;
  for (const auto& token : tokens) {
    if (token.type != ms::micros::TokenType::END_OF_FILE) {
      std::cout << "[" << ++count << "] " << token.toString() << std::endl;
    }
  }
  
  std::cout << "\n=== Lexing completed successfully! ===" << std::endl;
  return 0;
}
