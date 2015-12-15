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
            processLine(parsedFile[line]);
        }
        catch(const std::string &e)
        {
            std::cout << "\nError on line " << line+1 << ": " << e << std::endl;
            return false;
        }
        catch(const std::exception &e)
        {
            std::cout << "\nAn internal exception occurred whilst compiling line: "  << line+1 << std::endl;
            return false;
        }
        catch(...)
        {
            std::cout << "\nAn unknown error occurred whilst compiling line: "  << line+1 << std::endl;
            return false;
        }
    }

    BytecodeIO::writeBytecode("out.fry", &bytecode);
    return true;
}

void Compiler::processLine(const std::vector<std::string>& line)
{
    //Don't do anything if there's nothing in it
    if(line.empty())
        return;

    //Check to see what the line wants to do
    if(line.front() == "Console")
        processConsole(line);
    else if(stringToDataType(line.front()) != DataType::NIL || line.front() == "auto")
        processVariable(line);
    else if(line.front() == "if")
        processIF(line);
    else if((line.front() == "{" || line.front() == "}"))
        processScope(line);
    else if(isVariable(line.front()) != -1)
        processVariable(line);
    else
        throw std::string("Unknown instruction '" + line.front() + "'");
}

void Compiler::processScope(const std::vector<std::string> &line)
{
    //map<key><startDepth, beginPos>
    if(line[0] == "{") //Scope open
    {
        scopes[scopes.size()] = std::make_pair(scopeDepth, bytecode.size()); //Store current bytecode position for scope start in the

        scopeDepth++;
    }
    else if(line[0] == "}") //Scope close
    {
        scopeDepth--;
        if(scopeDepth < 0) //No scope to close!
        {
            throw std::string("No such scope to close");
        }

        //Find which scope it is ending
        for(auto iter = scopes.begin(); iter != scopes.end(); iter++)
        {
            if(iter->second.first == scopeDepth) //If a scope is ending and this is the scope we need to finish
            {
                bytecode[iter->second.second-1] = bytecode.size()-1;
                iter = scopes.erase(iter);
                break;
            }
        }
    }
    else
    {
        throw std::string("Error processing scope change, unknown operator '" + line[0] + "'");
    }
}

void Compiler::processIF(const std::vector<std::string>& line)
{
    evaluateBracket(line[1]);
    bytecode.emplace_back(Instruction::CONDITIONAL_IF);
    bytecode.emplace_back(0); //0 for now, we're just reserving space for it as the position will be set once the bytecode is compiled & length is known
}

void Compiler::validateArgumentCount(unsigned int expected, unsigned int got)
{
    if(expected != got)
    {
        throw std::string("Malformed argument list, got " + std::to_string(got) + " when expected " + std::to_string(expected));
    }
}

void Compiler::processVariable(const std::vector<std::string>& line) //things like "int a = 20;
{
    DataType possibleType = stringToDataType(line[0]);
    if(possibleType == DataType::NIL && line[0] != "auto") //If we're not creating a new variable (eg int a = 20)
    {
        evaluateBracket("(" + line[2] + ")"); //Evaluate the bracket containing what the variable should be set to
        bytecode.emplace_back(Instruction::SET_VARIABLE); //Add the set variable instructions
        bytecode.emplace_back(isVariable(line[0]));
    }
    else //If we're creating a new variable (eg int a = 20)
    {
        //Convert its data type from string to enum
        DataType type = stringToDataType(line[0]);
        if(line.size() > 2) //If there's a value provided (int a = 20)
        {
            //Convert initial value to bytecode
            evaluateBracket("(" + line[3] + ")");
        }
        else //Else no default value provided (int a)
        {
            //Give a default value based on its type
            if(line[0] == "string")
            {
                bytecode.emplace_back(Instruction::CREATE_STRING);
                bytecode.emplace_back(0);
            }
            else if(line[0] == "int")
            {
                bytecode.emplace_back(Instruction::CREATE_INT);
                bytecode.emplace_back(0);
            }
            else if(line[0] == "char")
            {
                bytecode.emplace_back(Instruction::CREATE_CHAR);
                bytecode.emplace_back(' ');
            }
            else if(line[0] == "bool")
            {
                bytecode.emplace_back(Instruction::CREATE_BOOL);
                bytecode.emplace_back(0);
            }
            else if(line[0] == "auto")
            {
                throw std::string("Can't deduct type for auto as no initial value is provided");
            }
        }

        //Add the variable create instruction
        variableStack.emplace_back(Variable(line[1], type));
    }
    return;
}

void Compiler::processConsole(const std::vector<std::string>& line)
{
    std::string operation = line[1];
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
        //Get operator of bracket if any
        const std::string &segmentOperator = segments.back();
        //Check to see if it's a variable, if so, float to top of stack
        Instruction segmentInstruction = stringToInstruction(segmentOperator);
        for(unsigned int a = 0; a < segments.size()-1; a++)
        {
            addVariableToStack(segments[a]);
        }
        //If it's an instruction
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
        else //Else if its a piece of data
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
    //Take off surrounding brackets if any
    if(originalLine[0] == '(')
        originalLine.erase(0,1);
    if(originalLine[originalLine.size()-1] == ')')
        originalLine.erase(originalLine.size()-1, 1);
    for(unsigned int c = 0; c < originalLine.size(); c++) //Ignore opening and close brackets
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
        while(arg[0] == ' ')
                arg.erase(0,1);
        while(arg[arg.size()-1] == ' ')
            arg.erase(arg.size()-1, 1);
        arg.insert(0, "(");
        arg += ")";
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







