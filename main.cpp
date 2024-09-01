#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "compiler.hpp"
#include "errors.hpp"

int main(int argc, char* argv[]) {
    // 1. extract paths from args
    if (argc != 4) {
        std::cerr << "Invalid usage: target -o output\n";
        exit(EXIT_FAILURE);
    }

    std::string inPath( argv[1] );
    std::string outPath( argv[3] );

    // 2. compile source files
    const std::string asmPath = outPath + ".asm";
    const std::string objPath = outPath + ".o";

    try {
        compileSrc(inPath, asmPath);
    } catch (DTException& e) {
        std::cout << e.what() << '\n';
    }

    // 3. invoke NASM to assemble
    std::system(("nasm -f elf64 " + asmPath).c_str());

    // 4. invoke GNU linker
    std::system(("ld " + objPath + " -o " + outPath).c_str());
    return 0;
}