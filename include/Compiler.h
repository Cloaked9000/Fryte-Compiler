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
        std::vector<Scope> scopes; //Keeps track of open scopes
        std::unordered_map<std::string, unsigned int> gotos; //Keeps track of created gotos. map<identifier, bytecodePos>
        int scopeDepth = 0; //Keeps track of current scope depth
        Scope expectedScopeType; //Keeps track of what the next expected scope type is

        unsigned int variablesOnStack = 0;

        void validateArgumentCount(unsigned int expected, unsigned int got);

        void processVariable(const std::vector<std::string>& line);

        void processGoto(const std::vector<std::string> &line);

        int isVariable(const std::string &identifier); //Returns the stack position of the variable if found, -1 otherwise

        void processLine(const std::vector<std::string> &line);

        void processConsole(const std::vector<std::string> &line); //Console.*

        void processIF(const std::vector<std::string> &line); //Conditional if statements

        void processScope(const std::vector<std::string> &line); //Process scope changes

        void processWhile(const std::vector<std::string> &line); //Processes a while loop

        unsigned int evaluateBracket(std::string line); //Converts bracket information into stack instructions. Returns the number of things added to the stack.
};

#endif // COMPILER_H
