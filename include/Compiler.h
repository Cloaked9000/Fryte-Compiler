#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

#include "Parser.h"
#include "BytecodeIO.h"
#include "VMTypes.h"

class Compiler
{
    public:
        Compiler();
        virtual ~Compiler();
        bool compile(std::vector<std::string> &data);
    protected:
    private:
        Parser parser;
        std::vector<unsigned char> bytecode; //Keeps track of bytecode to write to file
        std::vector<Variable> variableStack; //Keeps track of variable position on the stack to be
        std::unordered_map<unsigned int, std::pair<unsigned int, unsigned int>> scopes; //Keeps track of where each scope's start and end point in the bytecode are and its start depth. map<key><startDepth, beginPos>
        int scopeDepth = 0; //Keeps track of current scope depth

        unsigned int variablesOnStack = 0;

        void validateArgumentCount(unsigned int expected, unsigned int got);

        void processVariable(const std::vector<std::string>& line);

        int isVariable(const std::string &identifier); //Returns the stack position of the variable if found, -1 otherwise

        void processLine(const std::vector<std::string> &line);

        void processConsole(const std::vector<std::string> &line); //Console.*

        void processIF(const std::vector<std::string> &line); //Conditional if statements

        void processScope(const std::vector<std::string> &line); //Process scope changes

        unsigned int evaluateBracket(std::string line); //Converts bracket information into stack instructions. Returns the number of things added to the stack.
};

#endif // COMPILER_H
