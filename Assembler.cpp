// CS530 Assignment 2 - Two-Pass SIC/XE Assembler
// Team: Kristen Waterford (cssc2556, Red ID: 826655811)
//        Duncan Hugelmaier (cssc2523, Red ID: 131941547)

#include "Assembler.h"
#include "Parser.h"
#include "OpcodeTable.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <map>


//constructor for assembler
Assembler::Assembler() {
    reset();
}

//reset all variable before assembling new file
void Assembler::reset() {
    lines.clear();
    symtab.clear();
    littab.clear();
    pendingLits.clear();
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

        //collect literals from operands (operands starting with '=')
        if (!line.operand.empty()) {
            std::string rawOperand = line.operand;
            size_t commaPos = rawOperand.find(',');
            if (commaPos != std::string::npos) rawOperand = rawOperand.substr(0, commaPos);
            if (!rawOperand.empty() && (rawOperand[0] == '#' || rawOperand[0] == '@'))
                rawOperand = rawOperand.substr(1);
            if (!rawOperand.empty() && rawOperand[0] == '=') {
                bool found = false;
                for (const auto& lit : pendingLits)
                    if (lit == rawOperand) { found = true; break; }
                if (!found) pendingLits.push_back(rawOperand);
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
        } else if (opcodeUpper == "LTORG" || opcodeUpper == "*") {
            //flush pending literals at this point in the program
            for (const auto& lit : pendingLits) {
                if (!littab.count(lit)) {
                    littab[lit] = locctr;
                    std::string litBody = lit.substr(1); // strip leading '='
                    int len = getByteLength(litBody);
                    if (len > 0) locctr += len;
                }
            }
            pendingLits.clear();
        } else if (opcodeUpper == "END") {
            //flush any remaining pending literals 
            for (const auto& lit : pendingLits) {
                if (!littab.count(lit)) {
                    littab[lit] = locctr;
                    std::string litBody = lit.substr(1);
                    int len = getByteLength(litBody);
                    if (len > 0) locctr += len;
                }
            }
            pendingLits.clear();
            break;
        } else {
            line.errorMsg = "Invalid opcode: " + line.opcode;
        }
    }

    programLength = locctr - startAddress;
}

void Assembler::pass2() {
    // register name -> register number for format 2 instructions
    static const std::map<std::string, int> REGTAB = {
        {"A",0},{"X",1},{"L",2},{"B",3},{"S",4},{"T",5},{"F",6},{"PC",8},{"SW",9}
    };

    for (size_t i = 0; i < lines.size(); i++) {
        SourceLine& line = lines[i];

        if (line.isCommentLine || !line.errorMsg.empty()) continue;

        std::string opcodeRaw = line.opcode;
        std::string opcodeUpper = toUpper(opcodeRaw);

        // --- update BASE/NOBASE during pass2 ---
        if (opcodeUpper == "BASE") {
            std::string sym = toUpper(line.operand);
            auto it = symtab.find(sym);
            if (it != symtab.end()) {
                baseAddress = it->second;
                baseSet = true;
            }
            continue;
        }
        if (opcodeUpper == "NOBASE") {
            baseSet = false;
            continue;
        }

        // --- directives with no object code ---
        if (opcodeUpper == "START" || opcodeUpper == "END" ||
            opcodeUpper == "RESW" || opcodeUpper == "RESB" ||
            opcodeUpper == "LTORG") {
            continue;
        }

        // --- literal pool line (opcode is "*", operand is the literal e.g. =C'EOF') ---
        if (opcodeUpper == "*") {
            std::string litOp = line.operand;
            if (!litOp.empty() && litOp[0] == '=') litOp = litOp.substr(1);
            if (litOp.size() >= 3) {
                char type = static_cast<char>(std::toupper(static_cast<unsigned char>(litOp[0])));
                std::string val = litOp.substr(2, litOp.size() - 3);
                std::ostringstream oss;
                if (type == 'C') {
                    for (char c : val)
                        oss << std::uppercase << std::hex
                            << std::setw(2) << std::setfill('0')
                            << static_cast<unsigned int>(static_cast<unsigned char>(c));
                } else if (type == 'X') {
                    oss << std::uppercase << val;
                }
                line.objectCode = oss.str();
            }
            continue;
        }

        // --- WORD directive ---
        if (opcodeUpper == "WORD") {
            int val = 0;
            try { val = parseNumber(line.operand); } catch (...) {
                line.errorMsg = "Invalid WORD operand";
                continue;
            }
            std::ostringstream oss;
            // 24-bit two's complement
            unsigned int uval = static_cast<unsigned int>(val) & 0xFFFFFF;
            oss << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << uval;
            line.objectCode = oss.str();
            continue;
        }

        // --- BYTE directive ---
        if (opcodeUpper == "BYTE") {
            std::string operand = line.operand;
            if (operand.size() < 3) { line.errorMsg = "Invalid BYTE operand"; continue; }
            char type = static_cast<char>(std::toupper(static_cast<unsigned char>(operand[0])));
            std::string val = operand.substr(2, operand.size() - 3);
            std::ostringstream oss;
            if (type == 'C') {
                for (char c : val)
                    oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                        << (static_cast<unsigned int>(static_cast<unsigned char>(c)));
            } else if (type == 'X') {
                oss << std::uppercase << val;
            } else {
                line.errorMsg = "Invalid BYTE operand";
                continue;
            }
            line.objectCode = oss.str();
            continue;
        }

        // --- literal pool entry (label is '=' literal) ---
        // Handled as BYTE-like encoding; the label holds the literal value
        if (!line.label.empty() && line.label[0] == '=') {
            std::string litBody = line.label.substr(1);
            std::string operand = litBody;
            if (operand.size() >= 3) {
                char type = static_cast<char>(std::toupper(static_cast<unsigned char>(operand[0])));
                std::string val = operand.substr(2, operand.size() - 3);
                std::ostringstream oss;
                if (type == 'C') {
                    for (char c : val)
                        oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                            << (static_cast<unsigned int>(static_cast<unsigned char>(c)));
                } else if (type == 'X') {
                    oss << std::uppercase << val;
                }
                line.objectCode = oss.str();
            }
            continue;
        }

        // --- instructions ---
        bool extended = (!opcodeRaw.empty() && opcodeRaw[0] == '+');
        std::string mnemonic = extended ? toUpper(opcodeRaw.substr(1)) : opcodeUpper;

        auto optIt = getOPTAB().find(mnemonic);
        if (optIt == getOPTAB().end()) {
            line.errorMsg = "Unknown opcode: " + mnemonic;
            continue;
        }
        int opbyte = optIt->second;

        // --- Format 1 ---
        static const std::vector<std::string> fmt1 = {
            "FIX","FLOAT","HIO","NORM","SIO","TIO"
        };
        bool isFmt1 = false;
        for (const auto& m : fmt1) if (mnemonic == m) { isFmt1 = true; break; }
        if (isFmt1) {
            std::ostringstream oss;
            oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << opbyte;
            line.objectCode = oss.str();
            continue;
        }

        // --- Format 2 ---
        static const std::vector<std::string> fmt2 = {
            "ADDR","CLEAR","COMPR","DIVR","MULR","RMO",
            "SHIFTL","SHIFTR","SUBR","SVC","TIXR"
        };
        bool isFmt2 = false;
        for (const auto& m : fmt2) if (mnemonic == m) { isFmt2 = true; break; }
        if (isFmt2) {
            // operand is "r1,r2" or just "r1" for CLEAR/TIXR/SVC
            int r1 = 0, r2 = 0;
            std::string ops = toUpper(line.operand);
            size_t cp = ops.find(',');
            if (cp != std::string::npos) {
                auto it1 = REGTAB.find(ops.substr(0, cp));
                auto it2 = REGTAB.find(ops.substr(cp + 1));
                if (it1 == REGTAB.end() || it2 == REGTAB.end()) {
                    line.errorMsg = "Invalid register in format 2 operand";
                    continue;
                }
                r1 = it1->second;
                r2 = it2->second;
            } else {
                auto it1 = REGTAB.find(ops);
                if (it1 == REGTAB.end()) {
                    // SVC takes a numeric constant
                    try { r1 = parseNumber(line.operand); } catch (...) {
                        line.errorMsg = "Invalid format 2 operand";
                        continue;
                    }
                } else {
                    r1 = it1->second;
                }
                r2 = 0;
            }
            std::ostringstream oss;
            oss << std::uppercase << std::hex
                << std::setw(2) << std::setfill('0') << opbyte
                << std::setw(1) << r1
                << std::setw(1) << r2;
            line.objectCode = oss.str();
            continue;
        }

        // --- Format 3 / 4 ---
        // RSUB has no operand
        if (mnemonic == "RSUB") {
            std::ostringstream oss;
            // RSUB: opcode=0x4C, n=1,i=1 → byte1 = (0x4C & 0xFC) | 0x03 = 0x4F
            // nixbpe = 110000, disp = 0 → 4F0000
            oss << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << 0x4F0000;
            line.objectCode = oss.str();
            continue;
        }

        OperandInfo info = parseOperandMode(line.operand, line.address);
        if (!info.valid) {
            line.errorMsg = "Undefined symbol or literal: " + line.operand;
            continue;
        }

        int n = info.n, ni = info.i, x = info.x;
        int targetAddr = info.targetAddr;

        if (extended) {
            // Format 4: e=1, full 20-bit address
            int byte1 = (opbyte & 0xFC) | (n << 1) | ni;
            int byte2 = (x << 7) | (0 << 6) | (0 << 5) | (1 << 4) | ((targetAddr >> 16) & 0x0F);
            int byte3 = (targetAddr >> 8) & 0xFF;
            int byte4 = targetAddr & 0xFF;
            std::ostringstream oss;
            oss << std::uppercase << std::hex
                << std::setw(2) << std::setfill('0') << byte1
                << std::setw(2) << std::setfill('0') << byte2
                << std::setw(2) << std::setfill('0') << byte3
                << std::setw(2) << std::setfill('0') << byte4;
            line.objectCode = oss.str();
        } else {
            // Format 3
            int disp = 0;
            int b = 0, p = 0;

            if (info.immConst) {
                // #number — use value directly, no PC/base relative
                disp = targetAddr;
                b = 0; p = 0;
                if (disp < -2048 || disp > 4095) {
                    line.errorMsg = "Immediate constant out of range for format 3";
                    continue;
                }
            } else {
                // try PC-relative
                int pcDisp = targetAddr - (line.address + 3);
                if (pcDisp >= -2048 && pcDisp <= 2047) {
                    disp = pcDisp;
                    p = 1; b = 0;
                } else if (baseSet) {
                    // try base-relative
                    int bDisp = targetAddr - baseAddress;
                    if (bDisp >= 0 && bDisp <= 4095) {
                        disp = bDisp;
                        b = 1; p = 0;
                    } else {
                        line.errorMsg = "Displacement out of range for format 3";
                        continue;
                    }
                } else {
                    line.errorMsg = "Displacement out of range and no BASE set";
                    continue;
                }
            }

            int byte1 = (opbyte & 0xFC) | (n << 1) | ni;
            int byte2 = (x << 7) | (b << 6) | (p << 5) | (0 << 4) | ((disp >> 8) & 0x0F);
            int byte3 = disp & 0xFF;
            std::ostringstream oss;
            oss << std::uppercase << std::hex
                << std::setw(2) << std::setfill('0') << byte1
                << std::setw(2) << std::setfill('0') << byte2
                << std::setw(2) << std::setfill('0') << byte3;
            line.objectCode = oss.str();
        }
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

        outFile << "  " << std::left << std::setw(8) << line.objectCode
                << std::right << "  " << line.originalText;

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

// parses a format 3/4 operand string and resolves the target address.
// returns an OperandInfo with n/i/x flags set and targetAddr filled in.
Assembler::OperandInfo Assembler::parseOperandMode(const std::string& operand,
                                                    int lineAddr) const {
    OperandInfo info;
    std::string token = operand;

    // --- strip addressing mode prefix ---
    if (!token.empty() && token[0] == '#') {
        info.n = 0;
        info.i = 1;
        token = token.substr(1);
    } else if (!token.empty() && token[0] == '@') {
        info.n = 1;
        info.i = 0;
        token = token.substr(1);
    } else {
        info.n = 1;
        info.i = 1;
    }

    // --- strip ,X suffix ---
    size_t commaPos = token.find(',');
    if (commaPos != std::string::npos) {
        std::string modifier = toUpper(token.substr(commaPos + 1));
        if (modifier == "X") {
            info.x = 1;
        }
        token = token.substr(0, commaPos);
    }

    // --- resolve target address ---
    if (token.empty()) {
        info.valid = false;
        return info;
    }

    if (token[0] == '=') {
        // literal — look up in littab
        auto it = littab.find(token);
        if (it != littab.end()) {
            info.targetAddr = it->second;
        } else {
            info.valid = false;
        }
        return info;
    }

    // check if token is a plain integer constant (decimal or hex with leading $)
    bool isNumber = true;
    for (char c : token) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) {
        // immediate numeric constant e.g. #0, #4096
        info.targetAddr = std::stoi(token);
        info.immConst = true;
        return info;
    }

    // symbol — look up in symtab
    std::string symUpper = toUpper(token);
    auto it = symtab.find(symUpper);
    if (it != symtab.end()) {
        info.targetAddr = it->second;
    } else {
        info.valid = false;
    }

    return info;
}