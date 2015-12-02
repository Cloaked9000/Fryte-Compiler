#ifndef COMPILER_H
#define COMPILER_H

#include <vector>
#include <string>
#include <iostream>
#include <map>

//List of instructions supported by the VM
enum Instruction
{
    CONSOLE_OUT = 0, //Prints out value, CONSOLE_OUT(data)
    CREATE_INT = 1, //Pushes an integer to the stack, CREATE_INT(data)
    CREATE_CHAR = 2, //Pushes a character to the stack, CREATE_CHAR(data)
    CREATE_BOOL = 3, //Pushes a boolean to the stack, CREATE_BOOL(data)
    CREATE_STRING = 4, //Pushes a string to the stack, CREATE_STRING(length, data1, data2, etc)
    GOTO = 5, //Sets the current bytecode position, GOTO(stackDepth)
    CONSOLE_IN = 6, //Reads in a value, CONSOLE_IN(data)
    MATH_ADD = 7, //Adds two or more numbers. MATH_ADD(NumberOfThingsToAdd, data1, data2, etc)
    MATH_SUBTRACT = 8, //Subtracts two or more numbers. MATH_SUBTRACT(NumberOfThingsToAdd, data1, data2, etc)
    MATH_MULTIPLY = 9, //Multiplies two or more numbers together. MATH_MULTIPLY(NumberOfThingsToAdd, data1, data2, etc)
    MATH_DIVIDE = 10, //Divides two or more numbers. MATH_DIVIDE(NumberOfThingsToAdd, data1, data2, etc)
    MATH_MOD = 11, //Modulus two or more things. MATH_MOD(NumberOfThingsToAdd, data1, data2, etc)
    CLONE_TOP = 12, //Clones a variable's contents from a position in the stack to the top of the stack. CLONE_TOP(StackPos)
};

//List of data types which the virtual machine supports
enum DataType
{
    NIL = -1,
    INT = 0,
    CHAR = 1,
    BOOL = 2,
    STRING = 3,
};

class Compiler
{
    public:
        Compiler();
        virtual ~Compiler();

        std::vector<unsigned char> compileSource(const std::vector<std::string> &parsedFileIn);
    protected:
    private:
        int stringToInstruction(const std::string &operation); //Converts for example '+' into Instruction::MATH_ADD
        DataType stringToDataType(const std::string &type); //Converts for example "int" into DataType::INT
        void extractBracket(std::string bracket, std::vector<std::string> &results);
        void bracketToBytecode(std::string bracket);
        std::string bracketOperatorFix(const std::string &data);
        void compileBracket(const std::string &bracket);

        struct Variable
        {
            Variable(const std::string &name, unsigned int stackPos, DataType varType)
            {
                identifier = name;
                stackPosition = stackPos;
                type = varType;
            }
            DataType type;
            std::string identifier;
            unsigned int stackPosition;
        };

        std::vector<unsigned char> bytecode;
        std::vector<Variable> stack;
        int resultsAdded = 0; //Keep track of how many results should currently be on the stack for maths
};

#endif // COMPILER_H
