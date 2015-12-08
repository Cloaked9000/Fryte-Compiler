#ifndef VMTYPES_H
#define VMTYPES_H
//List of instructions which the virtual machine supports
enum Instruction
{
    NONE = -1, //Indicates not found, compiler side only
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
    CONCENTRATE_STRINGS = 13, //Concentrates strings together into a new string. CONCENTRATE_STRINGS(NumberOfStrings)
};

//List of data types which the virtual machine supports
enum class DataType
{
    NIL = -1,
    INT = 0,
    CHAR = 1,
    BOOL = 2,
    STRING = 3,
};

struct Variable
{
    Variable(const std::string &name, DataType varType)
    {
        identifier = name;
        type = varType;
    }
    DataType type;
    std::string identifier;
};

static Instruction stringToInstruction(const std::string& operation)
{
    if(operation == "+")
        return Instruction::MATH_ADD;
    else if(operation == "-")
        return Instruction::MATH_SUBTRACT;
    else if(operation == "*")
        return Instruction::MATH_MULTIPLY;
    else if(operation == "/")
        return Instruction::MATH_DIVIDE;
    else if(operation == "%")
        return Instruction::MATH_MOD;
    else if(operation == "@")
        return Instruction::CONCENTRATE_STRINGS;
    return NONE; //Not found
}

static DataType stringToDataType(const std::string& type)
{
    if(type == "int")
        return DataType::INT;
    else if(type == "string")
        return DataType::STRING;
    else if(type == "char")
        return DataType::CHAR;
    else if(type == "bool")
        return DataType::BOOL;
    return DataType::NIL;
}
#endif // VMTYPES_H
