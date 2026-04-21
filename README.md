# CS530 Assignment 2 - Two-Pass SIC/XE Assembler

## Team Members
- Kristen Waterford | cssc2556 | Red ID: 826655811
- Duncan Hugelmaier | cssc2523 | Red ID: 131941547

## Overview
A limited two-pass assembler for the XE variant of the SIC/XE family of machines.
Implements all XE assembler features and directives through section 2.3.4 of the text
(excludes EQU and ORG). Does not generate object code files; produces listing and symbol table files only.

## Files
- `main.cpp` — Entry point; handles command-line arguments and drives assembly per file
- `Assembler.h / Assembler.cpp` — Core two-pass assembler: pass1 (SYMTAB/LOCCTR), pass2 (object code generation), listing and symbol table output
- `Parser.h / Parser.cpp` — Parses each source line into label, opcode, operand, comment fields
- `OpcodeTable.h / OpcodeTable.cpp` — SIC/XE opcode table (mnemonic -> opcode byte)
- `Makefile` — Builds the `lxe` executable

## How to Build
```
make
```
Requires `g++` with C++17 support (GCC or compatible).

## How to Run
```
./lxe file1.sic file2.sic ...
```
For each input file `foo.sic`, the assembler produces:
- `foo.l` — Formatted listing file with line numbers, addresses, object code, and source
- `foo.st` — Symbol table file with all symbols and their hex addresses

## Supported Directives
`START`, `END`, `WORD`, `RESW`, `RESB`, `BYTE`, `BASE`, `NOBASE`, `LTORG`

## Supported Addressing Modes
Simple, immediate (`#`), indirect (`@`), indexed (`,X`), PC-relative, base-relative, format 4 (`+`)

## Test Files
Test source files are located in the `data/` directory (e.g. `data/P2sample.sic`).

## Development Environment
- OS: Windows / macOS
- IDE: Visual Studio Code
- Compiler: GCC (g++)
- Version Control: Git / GitHub
- Testing: Edoras (SDSU Linux servers)
