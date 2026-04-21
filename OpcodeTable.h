// CS530 Assignment 2 - Two-Pass SIC/XE Assembler
// Team: Kristen Waterford (cssc2556, Red ID: 826655811)
//        Duncan Hugelmaier (cssc2523, Red ID: 131941547)
#ifndef OPCODETABLE_H
#define OPCODETABLE_H

#include <string>
#include <map>

// Returns the SIC/XE opcode table mapping mnemonic -> opcode byte
const std::map<std::string, int>& getOPTAB();

#endif
