using namespace std;
#include <iostream>
#include "Assembler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lxe <file1.sic> <file2.sic> ...\n";
        std::cerr << "Error: No input files provided.\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::cout << "Assembling: " << argv[i] << std::endl;

        Assembler assembler;
        assembler.assemble(argv[i]);
    }
    return 0;
}