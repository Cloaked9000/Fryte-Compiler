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

int Compiler::isFunction(const std::string& identifier)
{
    for(unsigned int a = 0; a < functions.size(); a++)
    {
        if(functions[a].identifier == identifier)
            return a; //Found, return position
    }
    return -1; //Not found
}

bool Compiler::compile(std::vector<std::string> &data)
{
    //Reserve space for the entry point goto
    bytecode.emplace_back(Instruction::GOTO);
    bytecode.emplace_back(2); //Two by default, so even if no entry point is defined it wont go into a loop

    //Compile the data
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
            std::cout << "\nAn internal exception occurred whilst compiling line: "  << line+1 << "\n" << e.what() << std::endl;
            return false;
        }
        catch(...)
        {
            std::cout << "\nAn unknown error occurred whilst compiling line: "  << line+1 << std::endl;
            return false;
        }
    }

    //Save to file
    BytecodeIO::writeBytecode("out.fry", &bytecode);
    return true;
}

void Compiler::processLine(const std::vector<std::string>& line)
{
    //Don't do anything if there's nothing in it
    if(line.empty())
        return;

    //Check to see what the line wants to do
    if(line[0] == "Console")
        processConsole(line);
    else if((stringToDataType(line[0]) != DataType::NIL || line[0] == "auto") && !(line.size() > 2 && line[2][0] == '('))
        processVariable(line);
    else if(line[0] == "if")
        processIF(line);
    else if((line[0] == "{" || line[0] == "}"))
        processScope(line);
    else if(isVariable(line.front()) != -1)
        processVariable(line);
    else if(line[0] == "goto" || line[0][line[0].size()-1] == ':')
        processGoto(line);
    else if(line[0] == "while")
        processWhile(line);
    else if(line[0] == "else")
        processElse(line);
    else if(line[0] == "for")
        processFor(line);
    else if(isFunction(line[0]) != -1 || (line.size() > 2 && line[2][0] == '('))
        processFunction(line);
    else //Else unknown
        throw std::string("'" + line[0] + "' is undefined");
}

void Compiler::processGoto(const std::vector<std::string> &line)
{
    if(line[0] == "goto") //If going to a goto
    {
        bool gotoFound = false;
        for(auto iter = std::begin(gotos); iter != std::end(gotos) && !gotoFound; iter++) //Find matching goto
        {
            if(iter->first == line[1]) //Goto identifier matches this one
            {
                bytecode.emplace_back(Instruction::GOTO);
                bytecode.emplace_back(iter->second);
                gotoFound = true;
            }
        }

        if(!gotoFound) //No match found
            throw std::string("Couldn't goto undeclared goto '" + line[1] + "'");
    }
    else //Else if creating a goto
    {
        //Store the goto, excluding the colon at the end for the identifier
        gotos[line[0].substr(0, line[0].size()-1)] = bytecode.size();
    }
}

void Compiler::processScope(const std::vector<std::string> &line)
{
    //map<key><startDepth, beginPos>
    if(line[0] == "{") //Scope open
    {
        Scope newScope(Scope(expectedScopeType.statementPos, bytecode.size(), scopeDepth, variableStack.size(), expectedScopeType.incrementor, expectedScopeType.identifier, expectedScopeType.type));
        if(expectedScopeType.type == Scope::ELSE)
        {
            Scope &previousScope = pastScopes.back(); // Get preceding scope
            bytecode.insert(bytecode.begin() + previousScope.endPos++, Instruction::GOTO);
            bytecode.insert(bytecode.begin() + previousScope.endPos++, 0); //0 for now, this will be filled in later
            bytecode[previousScope.startPos-1] += 2;
        }
        else if(expectedScopeType.type == Scope::FUNCTION)
        {
            functionStack.emplace_back(newScope);
        }

        scopes.emplace_back(newScope);
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
        for(auto iter = scopes.begin(); iter != scopes.end();)
        {
            Scope &current = *iter;

            if(current.scopeDepth == scopeDepth) //We've found the scope that's ending
            {
                //Called to clear this scope's variables
                auto clearScopeVariables = [&]()
                {
                    //Remove variables created in the scope
                    bytecode.emplace_back(Instruction::STACK_WALK);
                    bytecode.emplace_back(current.stackSize);

                    //Remove variables registered with the compiler during this scope
                    unsigned int removeTotal = variableStack.size() - current.stackSize;
                    variableStack.erase(variableStack.end()-removeTotal, variableStack.end());
                };

                //Set the scope end position
                current.endPos = bytecode.size();

                //Handle the scope end differently depending on its type
                if(current.type == Scope::IF) //If an IF scope is ending
                {
                    clearScopeVariables();

                    //Set the bytecode position to seek to if the IF statement should not run
                    bytecode[current.startPos-1] = bytecode.size()-1;
                }
                else if(current.type == Scope::WHILE) //If a WHILE scope is ending
                {
                    clearScopeVariables();

                    //Add the instructions to loop back around to the beginning of the for
                    bytecode.emplace_back(Instruction::GOTO);
                    bytecode.emplace_back(current.statementPos);
                    bytecode[current.startPos-1] = bytecode.size()-1;
                }
                else if(current.type == Scope::FOR) //If a FOR scope is ending
                {
                    clearScopeVariables();

                    //Parse and insert increment instructions
                    std::vector<std::string> arguments = parser.extractBracketArguments(current.incrementor);
                    std::vector<std::vector<std::string>> initialisationArguments;
                    parser.tokenizeFile({arguments[0]}, initialisationArguments);

                    //Add the instructions to loop back around to the beginning of the for
                    processVariable(initialisationArguments[0]);
                    bytecode.emplace_back(Instruction::GOTO);
                    bytecode.emplace_back(current.statementPos);
                    bytecode[current.startPos-1] = bytecode.size()-1;
                }
                else if(current.type == Scope::ELSE)
                {
                    clearScopeVariables();

                    Scope &previousScope = pastScopes.back(); // Get preceding scope
                    bytecode[previousScope.endPos-1] = bytecode.size(); //Set the previous if's goto to this point after the else
                }
                else if(current.type == Scope::FUNCTION)
                {
                    // *variables are cleaned up by the caller, not at the end of the function!*

                    //Get scope to return to from function stack
                    Scope endingScope = functionStack.back();
                    functionStack.pop_back();

                    //Bring exit point to top
                    bytecode.emplace_back(Instruction::CLONE_TOP);
                    bytecode.emplace_back(endingScope.stackSize);

                    //Insert the dynamic goto IF it's not the program entry point
                    if(endingScope.identifier != "entry")
                        bytecode.emplace_back(Instruction::DYNAMIC_GOTO);
                }
                else
                {
                    throw std::string("Ending unknown scope type");
                }

                //Store the scope in pastScopes and erase it from currentScopes as it's no longer open
                pastScopes.emplace_back(*iter);
                iter = scopes.erase(iter);
                break;
            }
            else
            {
                iter++;
            }

        }
    }
    else
    {
        throw std::string("Error processing scope change, unknown operator '" + line[0] + "'");
    }
}

void Compiler::processFor(const std::vector<std::string>& line)
{
    //Extract arguments
    std::vector<std::string> arguments = parser.extractBracketArguments(line[1]);
    std::vector<std::vector<std::string>> initialisationArguments;

    //The for loop initialisation data must be parsed first and then passed to processVariable
    parser.tokenizeFile({arguments[0]}, initialisationArguments);
    processVariable(initialisationArguments[0]);

    //We set the statement position AFTER the loop initialisation, as we don't want the it to initialise every loop
    expectedScopeType.statementPos = bytecode.size();

    //Evaluate the condition of the loop
    evaluateBracket("(" + arguments[1] + ")");

    //Add the conditional IF instructions
    bytecode.emplace_back(Instruction::CONDITIONAL_IF);
    bytecode.emplace_back(0); //0 for now, we're just reserving space for it as the position will be set once the bytecode is compiled & length is known

    //Set the expected scope data
    expectedScopeType.incrementor = "(" + arguments[2] + ")";
    expectedScopeType.type = Scope::FOR;
}

void Compiler::processElse(const std::vector<std::string>& line)
{
    expectedScopeType.type = Scope::ELSE;
}

void Compiler::processWhile(const std::vector<std::string>& line)
{
    expectedScopeType.statementPos = bytecode.size();
    evaluateBracket(line[1]);
    bytecode.emplace_back(Instruction::CONDITIONAL_IF);
    bytecode.emplace_back(0); //0 for now, we're just reserving space for it as the position will be set once the bytecode is compiled & length is known
    expectedScopeType.type = Scope::WHILE;
}

void Compiler::processIF(const std::vector<std::string>& line)
{
    expectedScopeType.statementPos = bytecode.size();
    evaluateBracket(line[1]);
    bytecode.emplace_back(Instruction::CONDITIONAL_IF);
    bytecode.emplace_back(0); //0 for now, we're just reserving space for it as the position will be set once the bytecode is compiled & length is known
    expectedScopeType.type = Scope::IF;
}

void Compiler::validateArgumentCount(unsigned int expected, unsigned int got)
{
    if(expected != got)
    {
        throw std::string("Malformed argument list, got " + std::to_string(got) + " when expected " + std::to_string(expected));
    }
}

void Compiler::processFunction(const std::vector<std::string>& line)
{
    DataType possibleType = stringToDataType(line[0]);
    if(possibleType != DataType::NIL && line.size() > 2 && line[2][0] == '(') //If we're defining a function
    {
        //Add the function
        functions.emplace_back(Variable(line[1], possibleType));
        expectedScopeType.type = Scope::FUNCTION;
        expectedScopeType.identifier = line[1];
        expectedScopeType.incrementor = line[2];

        //If this is the program entry point, update the entry point goto position
        if(line[1] == "entry")
            bytecode[1] = bytecode.size();
    }
    else
    {
        //Find the function's starting point from lastScopes
        Scope *scope; //Scope we're calling
        auto scopeIter = std::find_if(pastScopes.begin(), pastScopes.end(), [&] (const Scope &next) {if(next.identifier == line[0]){return true;}return false;});
        if(scopeIter == pastScopes.end()) //If it wasn't found in last scopes
        {
            if(expectedScopeType.identifier == line[0]) //If we're recursively calling the current scope
            {
                scope = &expectedScopeType; //Set found scope pointer to current scope
            }
            else //Else not found
            {
                throw std::string("Failed to find entry point for '" + line[0] + "'");
            }
        }
        else
        {
            scope = &(*scopeIter); //Set found scope pointer to the correct past scope
        }

        //Add in the goto to set the bytecode position to the beginning of the function
        unsigned int stackSizeBeforeFunction = variableStack.size();

        //Create the exit point
        variableStack.emplace_back(Variable("functionEnd", DataType::INT));
        bytecode.emplace_back(Instruction::CREATE_INT);
        bytecode.emplace_back(bytecode.size()+3); //Add a bit for the stack walk below

        //Goto the function
        bytecode.emplace_back(Instruction::GOTO);
        bytecode.emplace_back(scope->startPos);


        //Insert cleaning code
        unsigned int removeTotal = (variableStack.size() - scope->stackSize) + stackSizeBeforeFunction;
        for(int a = variableStack.size()-removeTotal; a < removeTotal; a++)
        {
            std::cout << "\nRemoving: " << variableStack.back().identifier;
            variableStack.pop_back();
        }
       // variableStack.erase(variableStack.end()-removeTotal, variableStack.end());
        bytecode.emplace_back(Instruction::STACK_WALK);
        bytecode.emplace_back(pastScopes.back().stackSize);
    }
}

void Compiler::processVariable(const std::vector<std::string>& line) //things like "int a = 20"
{
    DataType possibleType = stringToDataType(line[0]);
    if(possibleType == DataType::NIL && line[0] != "auto") //If we're not creating a new variable (eg a = 20)
    {
        if(line[1] == "=") //If it's a set operation
        {
            //Evaluate the bracket containing what the variable should be set to
            evaluateBracket("(" + line[2] + ")");
        }
        else //Else if it's a math operation
        {
            //First move the variable that we're multiplying by to the top of the stack for the operation
            bytecode.emplace_back(Instruction::CLONE_TOP);
            bytecode.emplace_back(isVariable(line[0]));

            //Evaluate the bracket containing what the variable should be set to
            evaluateBracket("(" + line[2] + ")");

            //Do different things depending on the assignment operator
            if(line[1] == "*=")
            {
                //Multiple the last two things on the stack (the bracket and the variable)
                bytecode.emplace_back(Instruction::MATH_MULTIPLY);
                bytecode.emplace_back(2);
            }
            else if(line[1] == "/=")
            {
                //Divide the last two things on the stack (the bracket and the variable)
                bytecode.emplace_back(Instruction::MATH_DIVIDE);
                bytecode.emplace_back(2);
            }
            else if(line[1] == "%=")
            {
                //Divide the last two things on the stack (the bracket and the variable)
                bytecode.emplace_back(Instruction::MATH_MOD);
                bytecode.emplace_back(2);
            }
            else if(line[1] == "+=")
            {
                //Divide the last two things on the stack (the bracket and the variable)
                bytecode.emplace_back(Instruction::MATH_ADD);
                bytecode.emplace_back(2);
            }
            else if(line[1] == "-=")
            {
                //Divide the last two things on the stack (the bracket and the variable)
                bytecode.emplace_back(Instruction::MATH_SUBTRACT);
                bytecode.emplace_back(2);
            }
        }

        //Set the variable to the last thing on the stack
        bytecode.emplace_back(Instruction::SET_VARIABLE);
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
    unsigned int oldVariablesOnStack = variablesOnStack;
    variablesOnStack = 0;
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
            else if(data == "true") //If boolean
            {
                bytecode.emplace_back(Instruction::CREATE_BOOL);
                bytecode.emplace_back(1);
            }
            else if(data == "false") //If boolean
            {
                bytecode.emplace_back(Instruction::CREATE_BOOL);
                bytecode.emplace_back(0);
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

    //Split the arguments
    std::vector<std::string> &&arguments = parser.extractBracketArguments(originalLine);

    //Bracket all arguments
    for(auto &arg : arguments)
    {
        arg.insert(0, "(");
        arg += ")";
    }

    //Parse each argument separately
    for(auto iter = arguments.rbegin(); iter != arguments.rend(); iter++)
    {
        variablesOnStack = 0;
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
    variablesOnStack = oldVariablesOnStack + arguments.size();
    return 0;
}






