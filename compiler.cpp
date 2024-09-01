#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "errors.hpp"
#include "ast/ast.hpp"
#include "ast/ast_extractor.hpp"

void compileSrc(const std::string& inPath, const std::string& asmPath) {
    // open src file
    std::ifstream inHandle( inPath );
    if (!inHandle.is_open()) {
        std::cerr << "Failed to open source file: " << inPath << '\n';
        exit(EXIT_FAILURE);
    }

    // create empty asm file
    std::ofstream outHandle( asmPath );
    if (!outHandle.is_open()) {
        inHandle.close();
        std::cerr << "Failed to create assembly file: " << asmPath + '\n';
        exit(EXIT_FAILURE);
    }

    // 1. tokenize document via lexer
    // 1.A register file with global filesIndex in errors.hpp
    const int fileIndex = DTException::registerFile(inPath);
    std::string line;
    std::vector<Token> tokens;
    trace lineNum = 0;
    while (std::getline(inHandle, line)) {
        if (line.back() == '\r') line.pop_back(); // remove carriage return if present
        tokenize(line, tokens, ++lineNum, fileIndex);
    }

    // 2. build AST via parser
    for (const Token& token : tokens) {
        std::cout << token.raw << ' ';
    }
    std::cout << '\n';
    AST& ast = *buildAST(tokens);

    // 3. semantic analysis
    // TODO - perform semantic analysis check on AST, throws errors or simply does nothing if passed all checks
    // try {
    //     checkSemantics(ast);
    // } catch (DTException& e) {
    //     delete &ast;
    //     throw;
    // }

    // 4. generate assembly code
    generateASM(outHandle, ast);

    // close file handles & free mem
    inHandle.close();
    outHandle.close();
    delete &ast;
}