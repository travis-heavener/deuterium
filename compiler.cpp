#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "compiler.hpp"
#include "lexer.hpp"

void compileSrc(std::ifstream& inHandle, const std::string& asmPath) {
    // create empty asm file
    std::ofstream outHandle( asmPath );
    if (!outHandle.is_open()) {
        inHandle.close();
        std::cerr << "Failed to create assembly file: " << asmPath + '\n';
        exit(EXIT_FAILURE);
    }
    outHandle << "global _start\n_start:\n";

    // 1. tokenize document via lexer
    std::string line;
    std::vector<Token> tokens;
    while (std::getline(inHandle, line)) {
        if (line.back() == '\r') line.pop_back(); // remove carriage return if present
        tokenize(line, tokens);
    }

    // 2. build AST via parser
    for (const Token& token : tokens) {
        std::cout << token.raw << ' ';
    }
    std::cout << '\n';

    // 3. semantic analysis

    // 4. generate assembly code
}