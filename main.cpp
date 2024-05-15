#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "compiler.hpp"

int main(int argc, char* argv[]) {
    // 1. extract paths from args
    if (argc != 4) {
        std::cerr << "Invalid usage: target -o output\n";
        exit(EXIT_FAILURE);
    }

    std::ifstream inHandle( argv[1] );
    std::string outPath( argv[3] );

    if (!inHandle.is_open()) {
        std::cerr << "Failed to open source file: " << argv[1] << '\n';
        exit(EXIT_FAILURE);
    }

    // 2. compile source files
    const std::string asmPath = outPath + ".asm";
    const std::string objPath = outPath + ".o";
    compileSrc(inHandle, asmPath);

    // 3. invoke nasm to assemble
    std::system(("nasm -f elf64 " + asmPath).c_str());

    // 4. invoke Ubuntu linker
    std::system(("ld " + objPath + " -o " + outPath).c_str());
    return 0;
}