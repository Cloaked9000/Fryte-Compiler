#include "Compiler.h"

Compiler::Compiler()
{
    //ctor
}

Compiler::~Compiler()
{
    //dtor
}

int Compiler::isVariable(const std::string& identifier)
{
    for(unsigned int a = 0; a < variableStack.size(); a++)
    {
        if(variableStack[a].identifier == identifier)
            return a; //Found, return stack position
    }
    return -1; //Not found
}

bool Compiler::compile(std::vector<std::string> &data)
{
    std::vector<std::vector<std::string>> parsedFile;
    parser.tokenizeFile(data, parsedFile);
    for(unsigned int line = 0; line < parsedFile.size(); line++)
    {
        try
        {
            std::cout << "\nProcessing line " << line;
            processLine(parsedFile[line]);
        }
        catch(const std::string &e)
        {
            std::cout << "\nError on line " + line << ": " << e;
            return false;
        }
        catch(const std::exception &e)
        {
            std::cout << "\nAn internal exception occurred whilst compiling line: "  << line;
            return false;
        }
        catch(...)
        {
            std::cout << "\nAn unknown error occurred whilst compiling line: "  << line;
            return false;
        }
    }

    BytecodeIO::writeBytecode("out.fry", &bytecode);
    return true;
}

void Compiler::processLine(const std::vector<std::string>& line)
{
    for(unsigned int a = 0; a < line.size(); a++)
    {
        if(a == 0 && line[a] == "Console")
            processConsole(line);
        else if(a == 0 && (stringToDataType(line[a]) != DataType::NIL))
            processVariable(line);
        else if(a == 0 && line[a] == "if")
            processIF(line);
    }
}

void Compiler::processIF(const std::vector<std::string>& line)
{

}

void Compiler::validateArgumentCount(unsigned int expected, unsigned int got)
{
    if(expected != got)
    {
        throw std::string("Malformed argument list, got " + std::to_string(got) + " when expected " + std::to_string(expected));
    }
}

void Compiler::processVariable(const std::vector<std::string>& line)
{
    //Convert string type to Instruction
    evaluateBracket("(" + line[3] + ")");
    variableStack.emplace_back(Variable(line[1], stringToDataType(line[0])));
    return;
}

void Compiler::processConsole(const std::vector<std::string>& line)
{
    std::string operation = line[1];
    std::cout << "\nOperation: " << operation;
    std::string data = line[2];
    if(operation == "print")
    {
        unsigned int stackSizeOld = variablesOnStack;
        evaluateBracket(data);
        for(unsigned int a = stackSizeOld; a < variablesOnStack; a++)
            bytecode.emplace_back(Instruction::CONSOLE_OUT);
    }
    else if(operation == "scan")
    {
        std::vector<std::string> bracket;
        parser.extractBracket(data, bracket);
        int varPos = isVariable(bracket[0]);
        if(varPos == -1) //Error, variable not found
            throw std::string("Cannot scan into variable " + data + ", variable undeclared");
        else
        {
            bytecode.emplace_back(Instruction::CONSOLE_IN);
            bytecode.emplace_back(varPos);
        }
        std::cout << "\nDone!";
    }
    else
    {
        throw std::string("Unknown Console operation: " + operation);
    }
}

unsigned int Compiler::evaluateBracket(std::string originalLine)
{
    auto addVariableToStack = [&] (const std::string &data) -> void
    {
        int possibleVariable = isVariable(data);
        if(possibleVariable != -1) //If it's a variable
        {
            bytecode.emplace_back(Instruction::CLONE_TOP);
            bytecode.emplace_back(possibleVariable);
        }
        else //Else if it's raw data
        {
            if(data[0] == '"') //If string
            {
                bytecode.emplace_back(Instruction::CREATE_STRING);
                bytecode.emplace_back(data.size()-2); //-2 to remove the ""
                for(unsigned int a = 1; a < data.size()-1; a++)
                    bytecode.emplace_back(data[a]);
            }
            else if(data[0] == '\'') //If character
            {
                bytecode.emplace_back(Instruction::CREATE_CHAR);
                bytecode.emplace_back(data[1]);
            }
            else //Else if number
            {
                bytecode.emplace_back(Instruction::CREATE_INT);
                bytecode.emplace_back(stoi(data));
            }
        }
    };

    auto handleSubSegment = [&] (const std::vector<std::string> &segments) -> void
    {
        const std::string &segmentOperator = segments.back();
        Instruction segmentInstruction = stringToInstruction(segmentOperator);
        for(unsigned int a = 0; a < segments.size()-1; a++)
        {
            addVariableToStack(segments[a]);
        }
        if(segmentInstruction != Instruction::NONE)
        {
            bytecode.emplace_back(segmentInstruction);
            if(segments.size() == 1)
            {
                bytecode.emplace_back(variablesOnStack);
                variablesOnStack = 0;
            }
            else
            {
                bytecode.emplace_back(segments.size()-1);
            }
            variablesOnStack++;
        }
        else
        {
            if(segments.size() == 1)
            {
                addVariableToStack(segmentOperator);
                variablesOnStack++;
            }
            variablesOnStack += segments.size()-1;
        }
    };
    //Split each argument up first
    std::vector<std::string> arguments;
    std::string argumentBuffer;
    bool isQuoteOpen = false;
    for(unsigned int c = 1; c < originalLine.size()-1; c++) //Ignore opening and close brackets
    {
        if(originalLine[c] == '"')
            isQuoteOpen = !isQuoteOpen;
        if(originalLine[c] == ',' && !argumentBuffer.empty() && !isQuoteOpen)
        {
            arguments.emplace_back(argumentBuffer);
            argumentBuffer.clear();
        }
        else
        {
            argumentBuffer += originalLine[c];
        }
    }

    arguments.emplace_back(argumentBuffer);
    argumentBuffer.clear();

    //Process brackets in each argument
    for(auto &arg : arguments)
    {
        while(arg[0] == '(' || arg[0] == ' ')
                arg.erase(0,1);
        while(arg[arg.size()-1] == ')' || arg[arg.size()-1] == ' ')
            arg.erase(arg.size()-1, 1);
        arg.insert(0, "(");
        arg += ")";
        std::cout << "\nArgument: " << arg;
    }

    //Parse each argument separately
    for(auto iter = arguments.rbegin(); iter != arguments.rend(); iter++)
    {
        const std::string &lineToParse = *iter;
        std::string line = parser.bracketOperatorFix(lineToParse);
        std::vector<std::string> components;
        parser.extractBracket(line, components);

        //Split segments by space
        for(const auto &segment : components)
        {
            //Split by space
            std::string peiceBuffer;
            std::vector<std::string> subSegment;
            bool isQuoteOpen = false;
            for(unsigned int c = 0; c < segment.size(); c++)
            {
                if(segment[c] == '"')
                    isQuoteOpen = !isQuoteOpen;
                if(segment[c] == ' ' && isQuoteOpen == false)
                {
                    subSegment.emplace_back(peiceBuffer);
                    peiceBuffer.clear();
                }
                else
                {
                    peiceBuffer += segment[c];
                }
            }
            subSegment.emplace_back(peiceBuffer);
            handleSubSegment(subSegment);
        }
    }
    return 0;
}







