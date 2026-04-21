# CS530 Assignment 2 - Two-Pass SIC/XE Assembler
# Team: Kristen Waterford (cssc2556, Red ID: 826655811)
#        Duncan Hugelmaier (cssc2523, Red ID: 131941547)


CXX = g++
CXXFLAGS = -std=c++17 -Wall

lxe: main.o Assembler.o Parser.o OpcodeTable.o
	$(CXX) $(CXXFLAGS) -o lxe main.o Assembler.o Parser.o OpcodeTable.o

main.o: main.cpp Assembler.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Assembler.o: Assembler.cpp Assembler.h Parser.h OpcodeTable.h
	$(CXX) $(CXXFLAGS) -c Assembler.cpp

Parser.o: Parser.cpp Parser.h Assembler.h
	$(CXX) $(CXXFLAGS) -c Parser.cpp

OpcodeTable.o: OpcodeTable.cpp OpcodeTable.h
	$(CXX) $(CXXFLAGS) -c OpcodeTable.cpp

clean:
	rm -f *.o lxe
