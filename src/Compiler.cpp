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
            std::cout << "\nError on line " + line << ": " << e;
            return false;
        }
        catch(const std::exception &e)
        {
            std::cout << "\nAn internal exception occurred whilst compiling line: "  << line;
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
    }
}

void Compiler::processConsole(const std::vector<std::string>& line)
{
    if(line.size() < 2)
        throw std::string("Malformed console call.");

    std::string varName = line[1];
    std::string data = line[2];

    const auto printCount = evaluateBracket(data);

    for(unsigned int a = 0; a < printCount; a++)
        bytecode.emplace_back(Instruction::CONSOLE_OUT);
}

unsigned int Compiler::evaluateBracket(const std::string& line)
{
    //Lambda to sort a single bracket
    unsigned int bracketsOnStack = 0;
    auto evaluateSingleBracket = [&] (const std::string &bracket)
    {
        //Check to see if this 'bracket' is just an operator
        Instruction possibleOperator = stringToInstruction(bracket);
        if(possibleOperator != Instruction::NONE) //If this bracket just contains an instruction
        {
            //Add operation to bytecode
            bytecode.emplace_back(possibleOperator);
            bytecode.emplace_back(bracketsOnStack);
            bracketsOnStack = 1;
            return;
        }


        //Split the bracket into sections, split token = ' '
        std::vector<std::string> sections;
        std::string sectionBuffer;
        for(const auto &c : bracket)
        {
            if(c == ' ')
            {
                sections.emplace_back(sectionBuffer);
                sectionBuffer.clear();
            }
            else
            {
                sectionBuffer += c;
            }
        }
        sections.emplace_back(sectionBuffer);

        bracketsOnStack++;
        //Iterate through each token
        unsigned int sectionsOnStack = 0;
        for(const auto &section : sections)
        {
            unsigned int possibleVariable = isVariable(section);
            if(possibleVariable != -1) //If this section is a variable name
            {
                //Bring it to the top of the stack
                bytecode.emplace_back(Instruction::CLONE_TOP);
                bytecode.emplace_back(possibleVariable);
                sectionsOnStack++;
            }
            else
            {
                Instruction possibleOperator = stringToInstruction(section);
                if(possibleOperator != Instruction::NONE) //If this is an operator
                {
                    //Add operation to bytecode
                    bytecode.emplace_back(possibleOperator);
                    bytecode.emplace_back(sectionsOnStack);
                    sectionsOnStack = 0;
                }
                else //If this is a piece of data
                {
                    if(section[0] == '"') //If it's a string
                    {
                        //Create string on stack
                        bytecode.emplace_back(Instruction::CREATE_STRING);
                        bytecode.emplace_back(section.size());
                        for(const auto &c : section)
                           bytecode.emplace_back(c);
                        sectionsOnStack++;
                    }
                    else //If its not a string (let's assume integer)
                    {
                        //Create integer on stack
                        bytecode.emplace_back(Instruction::CREATE_INT);
                        bytecode.emplace_back(stoi(section));
                        sectionsOnStack++;
                    }
                }
            }

        }
    };

    std::string masterBracket = parser.bracketOperatorFix(line); //Might contain sub-brackets
    std::vector<std::string> splitBrackets; //Should contain a single bracket each
    parser.extractBracket(masterBracket, splitBrackets); //Extract master bracket into sub-brackets

    for(unsigned int a = 0; a < splitBrackets.size(); a++)
    {
        evaluateSingleBracket(splitBrackets[a]);
    }
    return bracketsOnStack;
}





