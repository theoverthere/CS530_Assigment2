#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "Assembler.h"

class Parser {
public:
    static void parseLine(SourceLine& line);

private:
    static std::string trim(const std::string& s);
    static bool isOpcodeOrDirective(const std::string& token);
};

#endif