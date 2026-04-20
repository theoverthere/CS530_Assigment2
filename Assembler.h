#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <map>

struct SourceLine {
    int lineNumber = 0;
    std::string originalText;

    std::string label;
    std::string opcode;
    std::string operand;
    std::string comment;

    bool isCommentLine = false;
    int address = -1;

    std::string objectCode;
    std::string errorMsg;
};

class Assembler {
public:
    Assembler();
    bool assemble(const std::string& filename);

private:
    std::vector<SourceLine> lines;
    std::map<std::string, int> symtab;

    int startAddress = 0;
    int programLength = 0;
    int baseAddress = 0;
    bool baseSet = false;

    bool readSourceFile(const std::string& filename);
    void pass1();
    void pass2();
    void writeListingFile(const std::string& filename);
    void writeSymtabFile(const std::string& filename);

    std::string makeOutputFilename(const std::string& inputFilename,
                                   const std::string& newExtension);
    void reset();

    bool isDirective(const std::string& opcode) const;
    bool isInstruction(const std::string& opcode) const;
    int getInstructionLength(const std::string& opcode) const;
    int getByteLength(const std::string& operand) const;
    int parseNumber(const std::string& text) const;
    std::string toUpper(const std::string& s) const;
};

#endif