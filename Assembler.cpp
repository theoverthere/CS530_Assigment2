#include "Assembler.h"
#include "Parser.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <vector>

//constructor for assembler
Assembler::Assembler() {
    reset();
}

//reset all variable before assembling new file
void Assembler::reset() {
    lines.clear();
    symtab.clear();
    startAddress = 0;
    programLength = 0;
    baseAddress = 0;
    baseSet = false;
}

//main assembler logic flow: read source file, pass1, pass2, write listing and symbol files
bool Assembler::assemble(const std::string& filename) {
    reset();

    if (!readSourceFile(filename)) {
        return false;
    }

    pass1();
    pass2();
    writeListingFile(filename);
    writeSymtabFile(filename);

    return true;
}

//reads the source file and stores each line in the lines vector
bool Assembler::readSourceFile(const std::string& filename) {
    std::ifstream inFile(filename);

    //error handling if file cannot be opened
    if (!inFile) {
        std::cerr << "Error: Could not open source file " << filename << std::endl;
        return false;
    }

    std::string lineText;
    int lineNumber = 1;

    while (std::getline(inFile, lineText)) {
        SourceLine line;
        line.lineNumber = lineNumber;
        line.originalText = lineText;

        //call parser to fill in the label, opcode, operand, and comment fields of the sourceline
        Parser::parseLine(line);

        //debugging
        std::cout << "Line " << line.lineNumber
          << " | label: [" << line.label << "]"
          << " opcode: [" << line.opcode << "]"
          << " operand: [" << line.operand << "]"
          << " comment: [" << line.comment << "]"
          << " isComment: " << line.isCommentLine
          << std::endl;

        lines.push_back(line);
        lineNumber++;
    }

    inFile.close();
    return true;
}

void Assembler::pass1() {
    int locctr = 0;
    bool startSeen = false;

    for (size_t i = 0; i < lines.size(); i++) {
        SourceLine& line = lines[i];

        //skip past comment lines
        if (line.isCommentLine) {
            continue;
        }

        std::string opcodeUpper = toUpper(line.opcode);

        //handle START opcode
        if (!startSeen && opcodeUpper == "START") {
            startSeen = true;

            if (!line.operand.empty()) {
                try {
                    startAddress = std::stoi(line.operand, nullptr, 16);
                    locctr = startAddress;
                } catch (...) {
                    line.errorMsg = "Invalid START address";
                    startAddress = 0;
                    locctr = 0;
                }
            } else {
                startAddress = 0;
                locctr = 0;
            }

            line.address = locctr;

            if (!line.label.empty()) {
                std::string labelUpper = toUpper(line.label);

                if (symtab.count(labelUpper)) {
                    if (line.errorMsg.empty()){
                        line.errorMsg = "Duplicate symbol: " + labelUpper;
                    }
                } else {
                    symtab[labelUpper] = locctr;
                }
            }

            continue;
        }

        if (!startSeen) {
            startAddress = 0;
            locctr = 0;
            startSeen = true;
        }

        line.address = locctr;

        //add label to symbol table if it exists, checking for duplicates
        if (!line.label.empty()) {
            std::string labelUpper = toUpper(line.label);
            if (symtab.count(labelUpper)) {
                if (line.errorMsg.empty()) {
                    line.errorMsg = "Duplicate symbol: " + labelUpper;
                }
            } else {
                symtab[labelUpper] = locctr;
            }
        }

        //advance LOCCTR based on opcode type
        if (isInstruction(line.opcode)) {
            locctr += getInstructionLength(line.opcode);
        } else if (opcodeUpper == "WORD") {
            locctr += 3;
        } else if (opcodeUpper == "RESW") {
            try {
                locctr += 3 * parseNumber(line.operand);
            } catch (...) {
                line.errorMsg = "Invalid operand for RESW";
            }
        } else if (opcodeUpper == "RESB") {
            try {
                locctr += parseNumber(line.operand);
            } catch (...) {
                line.errorMsg = "Invalid operand for RESB";
            }
        } else if (opcodeUpper == "BYTE") {
            int len = getByteLength(line.operand);
            if (len >= 0) {
                locctr += len;
            } else {
                line.errorMsg = "Invalid BYTE operand";
            }
        } else if (opcodeUpper == "BASE" || opcodeUpper == "NOBASE") {
            // no LOCCTR change
        } else if (opcodeUpper == "END") {
            break;
        } else {
            line.errorMsg = "Invalid opcode: " + line.opcode;
        }
    }

    programLength = locctr - startAddress;
}

void Assembler::pass2() {
    for (size_t i = 0; i < lines.size(); i++) {
        lines[i].objectCode = "";
    }
}

//creates the symbol table file with symbol and address
void Assembler::writeSymtabFile(const std::string& filename) {
    std::string outName = makeOutputFilename(filename, ".st");
    std::ofstream outFile(outName);

    if (!outFile) {
        std::cerr << "Error: Could not create symbol table file " << outName << std::endl;
        return;
    }

    outFile << "Symbol\tAddress\n";

    for (const auto& entry : symtab) {
        outFile << entry.first << "\t"
                << std::uppercase << std::hex << entry.second << std::dec
                << "\n";
    }

    outFile.close();
}

void Assembler::writeListingFile(const std::string& filename) {
    std::string outName = makeOutputFilename(filename, ".l");
    std::ofstream outFile(outName);

    if (!outFile) {
        std::cerr << "Error: Could not create listing file " << outName << std::endl;
        return;
    }

    for (const auto& line : lines) {
        outFile << std::setw(4) << line.lineNumber << "  ";

        if (!line.isCommentLine && line.address >= 0) {
            outFile << std::uppercase << std::hex << std::setw(4)
                    << std::setfill('0') << line.address
                    << std::setfill(' ') << std::dec;
        } else {
            outFile << "    ";
        }

        outFile << "  " << std::setw(8) << line.objectCode
                << "  " << line.originalText;

        if (!line.errorMsg.empty()) {
            outFile << "    *** " << line.errorMsg;
        }

        outFile << "\n";
    }

    outFile.close();
}

//helper function to change the file extension
std::string Assembler::makeOutputFilename(const std::string& inputFilename,
                                          const std::string& newExtension) {
    size_t dotPos = inputFilename.find_last_of('.');

    if (dotPos == std::string::npos) {
        return inputFilename + newExtension;
    }

    return inputFilename.substr(0, dotPos) + newExtension;
}

//helper function to convert a string to uppercase
std::string Assembler::toUpper(const std::string& s) const {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

//helper function to check if an opcode is a valid assembler directive
bool Assembler::isDirective(const std::string& opcode) const {
    std::string op = toUpper(opcode);

    return op == "START" || op == "END" || op == "BYTE" || op == "WORD" ||
           op == "RESB" || op == "RESW" || op == "BASE" || op == "NOBASE";
}

//helper function to check if an opcode is a valid instruction mnemonic
bool Assembler::isInstruction(const std::string& opcode) const {
    std::string op = toUpper(opcode);

    if (!op.empty() && op[0] == '+') {
        op = op.substr(1);
    }

    static const std::vector<std::string> format1 = {
        "FIX", "FLOAT", "HIO", "NORM", "SIO", "TIO"
    };

    static const std::vector<std::string> format2 = {
        "ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO",
        "SHIFTL", "SHIFTR", "SUBR", "SVC", "TIXR"
    };

    static const std::vector<std::string> format34 = {
        "ADD","ADDF","AND","COMP","COMPF","DIV","DIVF","J","JEQ","JGT","JLT",
        "JSUB","LDA","LDB","LDCH","LDF","LDL","LDS","LDT","LDX","LPS",
        "MUL","MULF","OR","RD","RSUB","SSK","STA","STB","STCH","STF","STI",
        "STL","STS","STSW","STT","STX","SUB","SUBF","TD","TIX","WD"
    };

    for (const auto& mnemonic : format1) {
        if (op == mnemonic) return true;
    }
    for (const auto& mnemonic : format2) {
        if (op == mnemonic) return true;
    }
    for (const auto& mnemonic : format34) {
        if (op == mnemonic) return true;
    }

    return false;
}

int Assembler::getInstructionLength(const std::string& opcode) const {
    std::string op = toUpper(opcode);
    bool extended = false;

    if (!op.empty() && op[0] == '+') {
        extended = true;
        op = op.substr(1);
    }

    static const std::vector<std::string> format1 = {
        "FIX", "FLOAT", "HIO", "NORM", "SIO", "TIO"
    };

    static const std::vector<std::string> format2 = {
        "ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO",
        "SHIFTL", "SHIFTR", "SUBR", "SVC", "TIXR"
    };

    for (const auto& mnemonic : format1) {
        if (op == mnemonic) return 1;
    }

    for (const auto& mnemonic : format2) {
        if (op == mnemonic) return 2;
    }

    if (isInstruction(op)) {
        return extended ? 4 : 3;
    }

    return 0;
}

int Assembler::parseNumber(const std::string& text) const {
    return std::stoi(text);
}

int Assembler::getByteLength(const std::string& operand) const {
    if (operand.size() < 3) {
        return -1;
    }

    if (operand[1] != '\'' || operand.back() != '\'') {
        return -1;
    }

    char type = static_cast<char>(std::toupper(static_cast<unsigned char>(operand[0])));
    std::string value = operand.substr(2, operand.size() - 3);

    if (type == 'C') {
        return static_cast<int>(value.size());
    }

    if (type == 'X') {
        if (value.size() % 2 != 0) {
            return -1;
        }
        return static_cast<int>(value.size() / 2);
    }

    return -1;
}