#include "Compiler.h"

Compiler::Compiler()
{
    //ctor
}

Compiler::~Compiler()
{
    //dtor
}

int Compiler::stringToInstruction(const std::string& operation)
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
    return -1; //Not found
}

DataType Compiler::stringToDataType(const std::string& type)
{
    if(type == "int")
        return DataType::INT;
    else if(type == "string")
        return DataType::STRING;
    else if(type == "char")
        return DataType::CHAR;
    else if(type == "bool")
        return DataType::BOOL;
    return DataType::NIL; //Not found
}

void Compiler::extractBracket(std::string bracket, std::vector<std::string> &results)
{
    //Ensure that data is not empty
    if(bracket.empty())
        return;

    //Now extract all bracket contents
    unsigned int bracketDepth = 0;
    unsigned int bracketBeginPos = 0;
    for(unsigned int a = 0; a < bracket.size(); a++)
    {
        //Each operator gets its own element in the results
        if(stringToInstruction(std::string(1, bracket[a])) != -1 && bracketDepth == 0)
            results.emplace_back(std::string(1, bracket[a]));

        //If another bracket layer is opening, keep track of the bracket depth
        else if(bracket[a] == '(')
        {
            if(bracketDepth == 0)
                bracketBeginPos = a+1;
            bracketDepth++;
        }
        //If the current bracket is closing, keep track of current depth
        else if(bracket[a] == ')')
        {
            bracketDepth--;
            if(bracketDepth == 0) //If we've reached the end of the first bracket we checked
            {
                //Extract the bracket
                std::string foundBracket = bracket.substr(bracketBeginPos, a-bracketBeginPos);
                size_t found = foundBracket.find('(');
                if(found == std::string::npos)
                    results.emplace_back(foundBracket); //Emplace data if the extracted bracket doesn't contain another bracket
                else
                    extractBracket(foundBracket, results); //If it does contain another bracket, recursively call extractBracket on it
            }

        }
    }
}

std::string Compiler::bracketOperatorFix(const std::string &data)
{
    //Convert things like "((1 + 2) + (3 * 4))" to "((1 2 +) (3 4 *) +)"

    std::string returnValue; //Store what the function will return as it's generated
    unsigned int currentLayer = 0; //To keep track of the current bracket layer
    std::map<unsigned int, char> bracketOperators; //Keep track of each bracket layer's operator. Map to keep track of multiple layers of brackets. map<LAYER, OPERATOR>

    //Find operators and data
    for(unsigned int a = 0; a < data.size(); a++)
    {
        //If new bracket, update bracket layer and set the layer's default operator (+)
        if(data[a] == '(')
        {
            currentLayer++;
            bracketOperators[currentLayer] = '+';
            returnValue += data[a];
        }

        //If we're exiting a bracket, update the layer and insert a space followed by this layer's operator
        else if(data[a] == ')')
        {
            returnValue += ' ';
            returnValue += bracketOperators[currentLayer];
            returnValue += data[a];
            currentLayer--;
        }

        //Else if an operator, don't add the operator to the return value and store this layer's operator
        else if(stringToInstruction(std::string(1, data[a])) != -1)
        {
            bracketOperators[currentLayer] = data[a];
            returnValue.erase(returnValue.size()-1, 1);
        }

        //Else, store current character. Not important.
        else
        {
            returnValue += data[a];
        }
    }
    return returnValue;
}

void Compiler::compileStringBracket(const std::string& bracket)
{
    //Convert quotations into brackets
    std::string bracketClone = bracket;
    bool bracketOpen = false;
    for(auto &c : bracketClone)
    {
        if(c == '"')
        {
            bracketOpen = !bracketOpen;
            if(bracketOpen)
            {
                c = '(';
            }
            else
            {
                c = ')';
            }
        }
    }

    std::vector<std::string> bracketSections;
    std::string bracketBuffer = bracketOperatorFix(bracketClone);
    extractBracket(bracketBuffer, bracketSections);
    for(auto &section : bracketSections)
    {
        //Compile bracket
        //Check to see if it's an operator
        int possibleOperator = stringToInstruction(section);
        if(possibleOperator != -1) //If it's an operator
        {
            if(possibleOperator == Instruction::MATH_ADD) //If '+'
            {
                bytecode.emplace_back(Instruction::CONCENTRATE_STRINGS);
                bytecode.emplace_back(resultsAdded);
                resultsAdded = 1;
            }
        }

        //Check to see if it's a variable
        std::string possibleVariableName = section.substr(0, section.size()-2); //-2 as processed variables come out as "varName +"
        bool isVariable = false;
        for(unsigned int a = 0; a < stack.size() && isVariable == false; a++)
        {
            if(stack[a].identifier == possibleVariableName) //If it's a variable
            {
                if(stack[a].type != DataType::STRING)
                    throw std::string("Unexpected data type found in bracket, expected string!");
                std::cout << "\nString variable found: " << possibleVariableName;
                isVariable = true;
                bytecode.emplace_back(Instruction::CLONE_TOP); //Float the variable to the top of the stack
                bytecode.emplace_back(stack[a].stackPosition);
                resultsAdded++;
            }
        }

        if(possibleOperator == -1 && isVariable == false) //Else it's a raw string
        {
            bytecode.emplace_back(Instruction::CREATE_STRING);
            bytecode.emplace_back(possibleVariableName.size());
            for(const auto &c : possibleVariableName)
            {
                bytecode.emplace_back(c);
            }
            resultsAdded++;
        }
    }

}

void Compiler::compileIntBracket(const std::string &wholeBracket)
{
    auto bracketToBytecode = [&](std::string bracket)
    {
        //If the bracket just contains an operator, add a matching math instruction
        int operatorType = stringToInstruction(bracket);
        if(operatorType != -1) //If it's a valid operator
        {
            bytecode.emplace_back(operatorType);
            bytecode.emplace_back(resultsAdded);
            resultsAdded = 1;
            return;
        }


        //Sort through more complex brackets
        std::string numberBuffer;
        int currentEquationSize = 0;
        for(const auto &c : bracket)
        {
            operatorType = stringToInstruction(std::string(1, c));
            if(c == ' ' && !numberBuffer.empty()) //We've found a chunk of information, add it as a variable or if its a variable bring it to the top of the stack
            {
                bool isVariable = false;
                //See if the section of the bracket contains a variable
                for(unsigned int a = 0; a < stack.size(); a++)
                {
                    if(stack[a].identifier == numberBuffer)
                    {
                        isVariable = true;
                        bytecode.emplace_back(Instruction::CLONE_TOP);
                        bytecode.emplace_back(stack[a].stackPosition);
                        currentEquationSize++;
                        a = stack.size();
                    }
                }
                if(isVariable == false) //If it's not a variable, then it's a number so add it to the stack
                {
                    currentEquationSize++;
                    bytecode.emplace_back(Instruction::CREATE_INT);
                    bytecode.emplace_back(stoi(numberBuffer));
                }
                numberBuffer.clear();
            }

            //If operator, add appropriate math instruction
            else if(operatorType != -1)
            {
                //Generate bytecode relevant to this operator
                bytecode.emplace_back(operatorType);
                resultsAdded++;

                //Store generated bytecode
                bytecode.emplace_back(currentEquationSize);
                numberBuffer.clear();
                currentEquationSize = 0;
            }
            else
            {
                numberBuffer += c;
            }
        }
    };

    std::vector<std::string> bracketSections;
    std::string bracketBuffer = bracketOperatorFix(wholeBracket);
    extractBracket(bracketBuffer, bracketSections);
    for(auto &section : bracketSections)
    {
        //Compile bracket
        bracketToBytecode(section);
    }
}

void Compiler::compileBracket(const std::string &bracket)
{
    resultsAdded = 0;
    bool isVariable = false;
    //First check to see if the bracket contains a variable
    std::vector<std::string> extractedBracket;
    extractBracket(bracket, extractedBracket);
    for(auto &a : extractedBracket)
        std::cout << "\nBracket: " << a;
    for(const auto &c : stack)
    {
        if(c.identifier == extractedBracket[0])
        {
            isVariable = true;
            if(c.type == DataType::STRING)
            {
                compileStringBracket(bracket);
            }
            else if(c.type == DataType::INT)
            {
                compileIntBracket(bracket);
            }
        }
    }

    if(isVariable == false) //If the bracket wansn't a variable, check to see if it is a string or not
    {
        if(bracket.find('"') == std::string::npos) //If not a string
        {
            compileIntBracket(bracket);
        }
        else //Else string
        {
            compileStringBracket(bracket);
        }
    }
}

unsigned int Compiler::bracketToStackPosition(const std::string& bracket)
{
    std::vector<std::string> results;
    extractBracket(bracket, results);
    //Find that variable in the stack
    for(const auto &c : stack)
    {
        if(c.identifier == results[0])
        {
            return c.stackPosition;
        }
    }
    return -1; //Failed
}

std::vector<unsigned char> Compiler::compileSource(const std::vector<std::string>& source)
{
    bytecode.clear();
    for(unsigned int a = 0; a < source.size(); a++)
    {
        if(stringToDataType(source[a]) != DataType::NIL) //If data type is being created
        {
            DataType type = stringToDataType(source[a]);

            std::string identifier = source[++a]; //Example: 'varName'

            stack.emplace_back(Variable(identifier, stack.size(), type));

            std::string operation = source[++a]; //Example: '+'
            if(operation == "=")
            {
                compileBracket(source[++a]);
            }
        }
        else if(source[a] == "Console")
        {
            std::string operation = source[++a]; //Example: 'scan' or 'print'
            if(operation == "print")
            {
                compileBracket(source[++a]);
                bytecode.emplace_back(Instruction::CONSOLE_OUT);
            }
            else if(operation == "scan")
            {
                bytecode.emplace_back(Instruction::CONSOLE_IN);
                bytecode.emplace_back(bracketToStackPosition(source[++a]));
            }
            else
            {
                throw std::string(source[a] + " has no operation named '" + operation + "'");
            }
        }
        else
        {
            throw std::string(source[a] + " was not declared in this scope");
        }
    }
    return bytecode;
}
