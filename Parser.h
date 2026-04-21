// CS530 Assignment 2 - Two-Pass SIC/XE Assembler
// Team: Kristen Waterford (cssc2556, Red ID: 826655811)
//        Duncan Hugelmaier (cssc2523, Red ID: 131941547)
#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "Assembler.h"

class Parser {
public:
    static void parseLine(SourceLine& line);

private:
    static std::string trim(const std::string& s);
};

#endif