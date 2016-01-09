#ifndef INSTRUCTIONGENERATOR_H
#define INSTRUCTIONGENERATOR_H

#include <string>
#include <vector>

#include "VirtualStack.h"
#include "VMTypes.h"

class InstructionGenerator : public VirtualStack
{
    public:
        InstructionGenerator(std::vector<unsigned char> *bytecodeOut);
        virtual ~InstructionGenerator();
        void genConsoleOut(unsigned int argCount);
        void genConsoleIn(const std::string &varDest);
        void genCreateInt(const std::string &identifier = "", unsigned int val = 0);
        void genCreateBool(const std::string &identifier = "", bool val = 0);
        void genCreateString(const std::string &identifier = "", const std::string &val = "");
        void genCreateChar(const std::string &identifier = "", const char val = 0);
        void genGoto(unsigned int pos);
        void genMathAdd(unsigned int argCount);
        void genMathSubtract(unsigned int argCount);
        void genMathDivide(unsigned int argCount);
        void genMathMultiply(unsigned int argCount);
        void genMathModulus(unsigned int argCount);
        void genCloneTop(const std::string &varName);
        void genCloneTop(int pos);
        void genConcentrateStrings(unsigned int argCount);
        void genCompareEqual(unsigned int argCount);
        void genConditionalIf(unsigned int skipToPos = 0);
        void genSetVariable(const std::string &varName);
        void genSetVariable(int offset);
        void genCompareUnequal(unsigned int argCount);
        void genCompareLessThan();
        void genCompareMoreThan();
        void genCompareLessThanOrEqual();
        void genCompareLessThanOrMore();
        void genCompareOr(unsigned int argCount);
        void genStackWalk(unsigned int pos);
        void genDynamicGoto();
        void genOperator(Instruction op, unsigned int argCount);
        void genToInteger(); //wip
        void genToString(); //wip
    protected:
    private:
        std::vector<unsigned char> *bytecode;
};

#endif // INSTRUCTIONGENERATOR_H
