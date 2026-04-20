#include "Parser.h"
#include <sstream>
#include <vector>
#include <cctype>

//trim leading and trailing whitespace from a given string
std::string Parser::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }

    return s.substr(start, end - start);
}

// //make sure the token is a valid opcode or directive, ignoring any leading '+' for format 4 instructions
// bool Parser::isOpcodeOrDirective(const std::string& token) {
//     static const std::vector<std::string> directives = {
//         "START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "NOBASE"
//     };

//     std::string check = token;

//     if (!check.empty() && check[0] == '+') {
//         check = check.substr(1);
//     }

//     for (const auto& dir : directives) {
//         if (check == dir) {
//             return true;
//         }
//     }

//     static const std::vector<std::string> opcodes = {
//         "ADD","ADDF","ADDR","AND","CLEAR","COMP","COMPF","COMPR",
//         "DIV","DIVF","DIVR","FIX","FLOAT","HIO","J","JEQ","JGT","JLT",
//         "JSUB","LDA","LDB","LDCH","LDF","LDL","LDS","LDT","LDX","LPS",
//         "MUL","MULF","MULR","NORM","OR","RD","RMO","RSUB","SHIFTL",
//         "SHIFTR","SIO","SSK","STA","STB","STCH","STF","STI","STL","STS",
//         "STSW","STT","STX","SUB","SUBF","SUBR","SVC","TD","TIO","TIX",
//         "TIXR","WD"
//     };

//     for (const auto& op : opcodes) {
//         if (check == op) {
//             return true;
//         }
//     }

//     return false;
// }

//parse source line one at a time, separating label, opcode, operand, and comment fields
void Parser::parseLine(SourceLine& line) {
    line.label.clear();
    line.opcode.clear();
    line.operand.clear();
    line.comment.clear();
    line.isCommentLine = false;

    std::string text = line.originalText;

    if (text.empty()) {
        line.isCommentLine = true;
        return;
    }

    std::string stripped = trim(text);

    if (stripped.empty()) {
        line.isCommentLine = true;
        return;
    }

    if (text[0] == '.') {
        line.isCommentLine = true;
        line.comment = text;
        return;
    }

    //habdle lines starting with *
    if (text[0] == '*') {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    line.label = "";
    if (!tokens.empty()) {
        line.opcode = tokens[0];
    }
    if (tokens.size() > 1) {
        line.operand = tokens[1];
    }
    if (tokens.size() > 2) {
        line.comment = tokens[2];
        for (size_t i = 3; i < tokens.size(); i++) {
            line.comment += " " + tokens[i];
        }
    }

    return;
    }

    bool hasLabel = !std::isspace(static_cast<unsigned char>(text[0]));

    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        line.isCommentLine = true;
        return;
    }

    if (hasLabel) {
    if (tokens.size() == 1) {
        line.label = tokens[0];
    } else if (tokens.size() == 2) {
        line.label = tokens[0];
        line.opcode = tokens[1];
    } else {
        line.label = tokens[0];
        line.opcode = tokens[1];
        line.operand = tokens[2];

        // everything after operand is comment
        if (tokens.size() > 3) {
            line.comment = tokens[3];
            for (size_t i = 4; i < tokens.size(); i++) {
                line.comment += " " + tokens[i];
            }
        }
    }
    } else {
    if (tokens.size() == 1) {
        line.opcode = tokens[0];
    } else {
        line.opcode = tokens[0];
        line.operand = tokens[1];

        // everything after operand is comment
        if (tokens.size() > 2) {
            line.comment = tokens[2];
            for (size_t i = 3; i < tokens.size(); i++) {
                line.comment += " " + tokens[i];
            }
        }
    }
}
}