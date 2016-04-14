#include "InstructionGenerator.h"
#include "Compiler.h"

InstructionGenerator::InstructionGenerator(std::vector<unsigned int> *bytecodeOut, Compiler *tcomp)
: bytecode(bytecodeOut), compiler(tcomp)
{
    //ctor
}

InstructionGenerator::~InstructionGenerator()
{
    //dtor
}

void InstructionGenerator::genConsoleOut(unsigned int argCount)
{
    std::cout << "\nConsole out count: " << argCount;
    bytecode->emplace_back(Instruction::CONSOLE_OUT);
    bytecode->emplace_back(argCount);
    for(unsigned int a = 0; a < argCount; a++)
        pop();
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
    {
        if(parser.isArrayDefinition(varName)) //Check if the variable is an array to calculate element offset
        {
            //Split array definition into name and size
            std::string arrayName;
            std::string arrayIndex;
            parser.splitArrayDefinition(varName, arrayName, arrayIndex);

            //Push array base position to stack
            genCreateInt("", static_cast<unsigned int>(isVariable(arrayName)+1));

            //Push index value to stack
            compiler->evaluateBracket("(" + arrayIndex + ")");

            //Subtract them to calculate offset
            genMathSubtract(2);

            //Dynamic clone top the calculated offset to the top
            genDynamicCloneTop();
        }
        else //Else if not an array element
        {
            //Check if we're moving a whole array
            Variable &var = getVariable(varPos);
            if(var.isArray) //We're moving a whole array
            {
                for(unsigned int a = 0; a < var.arrayLength; a++)
                {
                    genCloneTop(varPos); //We can keep using varPos repeatedly as the offset of the next variable is changed by pushing more things to the stack
                }
            }
            else //We're moving a single variable
            {
                genCloneTop(varPos);
            }
        }
    }
}

void InstructionGenerator::genDynamicCloneTop()
{
    bytecode->emplace_back(Instruction::DYNAMIC_CLONE_TOP);
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
    if(parser.isArrayDefinition(varName)) //Check if the variable is an array to calculate element offset
    {
        //Split array definition into name and size
        std::string arrayName;
        std::string arrayIndex;
        parser.splitArrayDefinition(varName, arrayName, arrayIndex);

        //Push array base position to stack
        genCreateInt("", static_cast<unsigned int>(isVariable(arrayName)));

        //Push index value to stack
        compiler->evaluateBracket("(" + arrayIndex + ")");

        //Subtract them to calculate offset
        genMathSubtract(2);

        //Dynamically set the variable at this positio
        genDynamicSetVariable();
    }
    else //Not an array, get variable position the usual way
    {
        genSetVariable(isVariable(varName));
    }
}

void InstructionGenerator::genDynamicSetVariable()
{
    bytecode->emplace_back(Instruction::DYNAMIC_SET_VARIABLE);
    pop();
    pop();
}

void InstructionGenerator::genSetVariable(int offset)
{
    bytecode->emplace_back(Instruction::SET_VARIABLE);
    bytecode->emplace_back(offset);
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
    if(pos == 0) //If no point in resizing, don't do it
        return;

    bytecode->emplace_back(Instruction::STACK_WALK);
    bytecode->emplace_back(pos);
    resize(pos);
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

void InstructionGenerator::genCreateDefaultValue(const std::string &identifier, DataType type)
{
    switch(type)
    {
    case DataType::BOOL:
        genCreateBool(identifier);
    break;
    case DataType::CHAR:
        genCreateChar(identifier);
        break;
    case DataType::INT:
        genCreateInt(identifier);
        break;
    case DataType::STRING:
        genCreateString(identifier);
        break;
    default:
        throw std::string("Unknown type passed to pushDefaultType!");
    }
}

void InstructionGenerator::genCreateArray(const std::string& identifier, const std::string& arraySize, DataType type)
{
    //Quick error check
    if(isVariable(arraySize) > 0)
    {
        throw std::string("Arrays of a dynamic size are not currently supported sorry!");
    }
    unsigned int aSize = std::stoull(arraySize);
    if(aSize == 0)
        throw std::string("Array must not contain no elements");

    //Create first element with a name
    genCreateDefaultValue(identifier, type);

    //Set it to an array
    Variable &var = getVariable(getStackSize()-1);
    var.isArray = true;
    var.arrayLength = aSize;

    //All elements after are unnamed so that when searching for the array, the first element will always be returned
    for(unsigned int a = 0; a < aSize-1; a++)
        genCreateDefaultValue("", type);
}

