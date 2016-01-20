#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <unordered_map>
#include <iterator>
#include <algorithm>

#include "Parser.h"
#include "BytecodeIO.h"
#include "VMTypes.h"
#include "InstructionGenerator.h"

class Compiler
{
    public:
        Compiler();
        virtual ~Compiler();
        bool compile(std::vector<std::string> &data);
    protected:
    private:
        Parser parser;
        InstructionGenerator igen;
        std::vector<unsigned int> bytecode; //Keeps track of bytecode to write to file
        std::vector<std::pair<unsigned int, unsigned int>> functionEndGotos; //Keep track of return gotos that still need to be set
        std::vector<Scope> scopes; //Keeps track of open scopes
        std::vector<Scope> pastScopes; //Keep a log of past scopes
        std::vector<Scope> functionStack; //Keeps track of function depth
        std::vector<Variable> functions; //Keeps track of defined functions
        std::unordered_map<std::string, unsigned int> gotos; //Keeps track of created gotos. map<identifier, bytecodePos>
        int scopeDepth = 0; //Keeps track of current scope depth
        Scope expectedScopeType; //Keeps track of scopes

        unsigned int line; //Current line being compiled

        void validateArgumentCount(unsigned int expected, unsigned int got);

        void processReturn(const std::vector<std::string>& line);

        int isFunction(const std::string& identifier);

        void processVariable(const std::vector<std::string>& line);

        void processFunction(const std::vector<std::string>& line, bool destroyReturnValue = true);

        void processGoto(const std::vector<std::string> &line);

        void processLine(const std::vector<std::string> &line);

        void processConsole(const std::vector<std::string> &line); //Console.*

        void processIF(const std::vector<std::string> &line); //Conditional if statements

        void processElse(const std::vector<std::string> &line); //Process else statements

        void processScope(const std::vector<std::string> &line); //Process scope changes

        void processWhile(const std::vector<std::string> &line); //Process a while loop

        void processFor(const std::vector<std::string> &line); //Process a for loop

        unsigned int evaluateBracket(std::string line); //Converts bracket information into stack instructions. Returns the number of things added to the stack.

        void displayWarning(const std::string &message);

        Scope *getPastScope(const std::string &identifier); //Returns a pointer to a past scope. Pointer to a vector so DO NOT store for long.
};

#endif // COMPILER_H
