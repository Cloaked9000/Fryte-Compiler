#include "InstructionGenerator.h"

InstructionGenerator::InstructionGenerator(std::vector<unsigned char> *bytecodeOut)
: bytecode(bytecodeOut)
{
    //ctor
}

InstructionGenerator::~InstructionGenerator()
{
    //dtor
}

void InstructionGenerator::genConsoleOut(unsigned int argCount)
{
    for(unsigned int a = 0; a < argCount; a++)
    {
        bytecode->emplace_back(Instruction::CONSOLE_OUT);
        pop();
    }
}

void InstructionGenerator::genConsoleIn(const std::string &varDest)
{
    bytecode->emplace_back(Instruction::CONSOLE_IN);
    bytecode->emplace_back(isVariable(varDest));
}

void InstructionGenerator::genCreateInt(const std::string &identifier, unsigned int val)
{
    bytecode->emplace_back(Instruction::CREATE_INT);
    bytecode->emplace_back(val);
    push(Variable(identifier, DataType::INT));
}

void InstructionGenerator::genCreateBool(const std::string &identifier, bool val)
{
    bytecode->emplace_back(Instruction::CREATE_BOOL);
    bytecode->emplace_back(val);
    push(Variable(identifier, DataType::BOOL));
}

void InstructionGenerator::genCreateString(const std::string &identifier, const std::string &val)
{
    bytecode->emplace_back(Instruction::CREATE_STRING);
    bytecode->emplace_back(val.size());
    for(const auto &c : val)
        bytecode->emplace_back(c);
    push(Variable(identifier, DataType::STRING));
}

void InstructionGenerator::genCreateChar(const std::string &identifier, const char val)
{
    bytecode->emplace_back(Instruction::CREATE_CHAR);
    bytecode->emplace_back(val);
    push(Variable(identifier, DataType::CHAR));
}

void InstructionGenerator::genGoto(unsigned int pos)
{
    bytecode->emplace_back(Instruction::GOTO);
    bytecode->emplace_back(pos);
}

void InstructionGenerator::genMathAdd(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::MATH_ADD);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::INT));
}

void InstructionGenerator::genMathSubtract(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::MATH_SUBTRACT);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::INT));
}

void InstructionGenerator::genMathDivide(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::MATH_DIVIDE);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::INT));
}

void InstructionGenerator::genMathMultiply(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::MATH_MULTIPLY);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::INT));
}

void InstructionGenerator::genMathModulus(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::MATH_MOD);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::INT));
}

void InstructionGenerator::genCloneTop(const std::string &varName)
{
    int varPos = isVariable(varName);
    if(varPos < 0)
        throw std::string("Undefined variable '" + varName + "'");
    else
        genCloneTop(varPos);
}

void InstructionGenerator::genCloneTop(int pos)
{
    bytecode->emplace_back(Instruction::CLONE_TOP);
    bytecode->emplace_back(pos);
    push(getVariable(pos));
}

void InstructionGenerator::genConcentrateStrings(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::CONCENTRATE_STRINGS);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::STRING));
}

void InstructionGenerator::genCompareEqual(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::COMPARE_EQUAL);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genConditionalIf(unsigned int skipToPos)
{
    bytecode->emplace_back(Instruction::CONDITIONAL_IF);
    bytecode->emplace_back(skipToPos);
    pop();
}

void InstructionGenerator::genSetVariable(const std::string &varName)
{
    bytecode->emplace_back(Instruction::SET_VARIABLE);
    bytecode->emplace_back(isVariable(varName));
    pop();
}

void InstructionGenerator::genCompareUnequal(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::COMPARE_UNEQUAL);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genCompareLessThan()
{
    bytecode->emplace_back(Instruction::COMPARE_LESS_THAN);
    bytecode->emplace_back(0);
    pop();
    pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genCompareMoreThan()
{
    bytecode->emplace_back(Instruction::COMPARE_MORE_THAN);
    bytecode->emplace_back(0);
    pop();
    pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genCompareLessThanOrEqual()
{
    bytecode->emplace_back(Instruction::COMPARE_LESS_THAN_OR_EQUAL);
    bytecode->emplace_back(0);
    pop();
    pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genCompareLessThanOrMore()
{
    bytecode->emplace_back(Instruction::COMPARE_MORE_THAN_OR_EQUAL);
    bytecode->emplace_back(0);
    pop();
    pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genCompareOr(unsigned int argCount)
{
    bytecode->emplace_back(Instruction::COMPARE_OR);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
    push(Variable("", DataType::BOOL));
}

void InstructionGenerator::genStackWalk(unsigned int pos)
{
    bytecode->emplace_back(Instruction::STACK_WALK);
    bytecode->emplace_back(pos);
}

void InstructionGenerator::genDynamicGoto()
{
    bytecode->emplace_back(Instruction::DYNAMIC_GOTO);
    pop();
}

void InstructionGenerator::genOperator(Instruction op, unsigned int argCount)
{
    DataType type;
    bytecode->emplace_back(op);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
    {
        type = pop().type;
    }
    push(Variable("", type));
}
