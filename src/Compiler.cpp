#include "Compiler.h"

Compiler::Compiler()
: igen(&bytecode, this)
{
    //ctor
}

Compiler::~Compiler()
{
    //dtor
}

bool Compiler::compile(std::vector<std::string> &data, std::vector<unsigned int> &output)
{
    //Write the entry point goto. Will be overwritten if 'entry' is defined. 2 to skip over the goto.
    igen.genGoto(2);

    //Compile the data
    std::vector<std::vector<std::string>> parsedFile;
    parser.tokenizeFile(data, parsedFile);
    for(line = 0; line < parsedFile.size(); line++)
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
            std::cout << "\nAn internal exception occurred whilst compiling line: "  << line+1 << "\nInformation: " << e.what() << std::endl;
            return false;
        }
        catch(...)
        {
            std::cout << "\nAn unknown error occurred whilst compiling line: " << line+1 << std::endl;
            return false;
        }
    }

    //Store result
    output = bytecode;
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
    else if((stringToDataType(line[0]) != DataType::UNKNOWN || line[0] == "auto") && !(line.size() > 2 && line[2][0] == '('))
        processVariable(line);
    else if(line[0] == "if")
        processIF(line);
    else if((line[0] == "{" || line[0] == "}"))
        processScope(line);
    else if(line[0] == "class")
        processClassDefinition(line);
    else if(igen.isVariable(line.front()) != -1)
        processVariable(line);
    else if(line[0] == "goto" || line[0][line[0].size()-1] == ':')
        processGoto(line);
    else if(line[0] == "while")
        processWhile(line);
    else if(line[0] == "else")
        processElse(line);
    else if(line[0] == "for")
        processFor(line);
    else if(getPastScope(line[0]) != nullptr || (line.size() > 2 && line[2][0] == '('))
        processFunction(line);
    else if(line[0] == "return")
        processReturn(line);
    else //Else unknown
        throw std::string("'" + line[0] + "' is undefined");
}

void Compiler::processReturn(const std::vector<std::string>& line)
{
    //Get ending function
    if(functionCallStack.empty())
        throw std::string("No function to escape from.");
    Scope &ending = functionCallStack.back();
    if(line.size() == 1 && ending.returnType != DataType::VOID) //If no return value is provided and the function is not nil
    {
        throw std::string("Expected return value, none given.");
    }
    if(ending.returnType == DataType::VOID && line.size() > 1) //If too many arguments
    {
        throw std::string("Function type is void, no return value expected.");
    }

    //Evaluate the return type if any
    if(ending.returnType != DataType::VOID)
    {
        evaluateBracket(parser.combineArguments(line, 1, line.size()-1));
        igen.genSetVariable((igen.getStackSize() - ending.stackSize) + 1);
    }

    igen.genGoto(0); //0 For now, will be filled in once function end point is known
    functionEndGotos.emplace_back(std::make_pair(functionCallStack.size()-1, bytecode.size()-1));
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
                igen.genGoto(iter->second);
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

void Compiler::processClassDefinition(const std::vector<std::string>& line)
{
    expectedScopeType.reset();
    expectedScopeType.identifier = line[1];
    expectedScopeType.type = Scope::CLASS;
}

void Compiler::processClassMemberAccess(const std::vector<std::string>& line)
{

}

void Compiler::processScope(const std::vector<std::string> &line)
{
    if(line[0] == "{") //Scope open
    {
        //Setup the new scope, taking details from the expected scope and filling in the '{' dependent stuff
        Scope newScope = expectedScopeType;
        newScope.startPos = bytecode.size();
        newScope.scopeDepth = scopeDepth;
        newScope.stackSize = igen.getStackSize();

        if(expectedScopeType.type == Scope::ELSE)
        {
            Scope &previousScope = pastScopes.back(); // Get preceding scope
            bytecode.insert(bytecode.begin() + previousScope.endPos++, Instruction::GOTO);
            bytecode.insert(bytecode.begin() + previousScope.endPos++, bytecode.size()+1); //Temporary value, this will be filled in later.
            bytecode[previousScope.startPos-1] += 2;
        }
        else if(expectedScopeType.type == Scope::FUNCTION)
        {
            std::vector<std::vector<std::string>> argumentNames;
            parser.tokenizeFile(parser.extractBracketArguments(newScope.incrementor), argumentNames);
            for(auto iter = argumentNames.rbegin(); iter != argumentNames.rend(); iter++)
            {
                DataType type = stringToDataType((*iter)[0]);
                std::string &identifier = (*iter)[1];
                if(parser.isArrayDefinition(identifier)) //Check if it's an array
                {
                    //Split array definition into name and size
                    std::string arrayName;
                    std::string arraySize;
                    parser.splitArrayDefinition(identifier, arrayName, arraySize);

                    //Quick error check
                    if(igen.isVariable(arraySize) > 0)
                        throw std::string("Arrays of a dynamic size are not currently supported sorry!");

                    //Get array size as an integer
                    unsigned int aSize = std::stoull(arraySize);
                    if(aSize == 0)
                        throw std::string("Array must not contain no elements");

                    //Create first element with a name
                    igen.push(Variable(arrayName, type));

                    //Set it to an array
                    Variable &var = igen.getVariable(igen.getStackSize()-1);
                    var.isArray = true;
                    var.arrayLength = aSize;

                    //All elements after are unnamed so that when searching for the array, the first element will always be returned
                    for(unsigned int a = 0; a < aSize-1; a++)
                        igen.push(Variable("", type));
                }
                else //Else normal variable
                {
                    igen.push(Variable(identifier, type));
                }
            }
            newScope.argumentCount = argumentNames.size();
            functionCallStack.emplace_back(newScope);
        }
        openScopeStack.emplace_back(newScope);
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
        for(auto iter = openScopeStack.begin(); iter != openScopeStack.end();)
        {
            Scope &current = *iter;

            if(current.scopeDepth == scopeDepth) //We've found the scope that's ending
            {
                if(current.startPos == bytecode.size() && current.type != Scope::FUNCTION) //If the scope was empty, optimise it out
                {
                    bytecode.erase(bytecode.begin() + current.bytecodeSizeBefore, bytecode.end());
                    current.wasOptimisedOut = true;
                    current.endPos = bytecode.size();
                    pastScopes.emplace_back(current);
                    displayWarning("Empty scope. It has been optimised out, but removal is recommended.");
                    return;
                }


                //Called to clear this scope's variables
                auto clearScopeVariables = [&](unsigned int extraAmount = 0)
                {
                    //Remove variables created in the scope IF there's any
                    if(igen.getStackSize() != current.stackSize || extraAmount > 0)
                    {
                        igen.genStackWalk((igen.getStackSize() - current.stackSize) + extraAmount);
                    }
                };

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

                    //Add the instructions to loop back around to the beginning of the while
                    igen.genGoto(current.statementPos);
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
                    igen.genGoto(current.statementPos);
                    bytecode[current.startPos-1] = bytecode.size()-1;

                    //Remove initialization variables if needed
                    igen.genStackWalk(current.initialisationVariableCount);
                }
                else if(current.type == Scope::ELSE)
                {
                    clearScopeVariables();

                    Scope &previousScope = pastScopes.back(); // Get preceding scope
                    if(!previousScope.wasOptimisedOut)
                        bytecode[previousScope.endPos-1] = bytecode.size(); //Set the previous if's goto to this point after the else
                }
                else if(current.type == Scope::FUNCTION)
                {
                    //Get scope to return to from function stack
                    Scope endingScope = functionCallStack.back();
                    functionCallStack.pop_back();

                    //Set the function return gotos for this scope
                    while(!functionEndGotos.empty() && functionEndGotos.back().first == functionCallStack.size())
                    {
                        bytecode[functionEndGotos.back().second] = bytecode.size();
                        functionEndGotos.pop_back();
                    }

                    //Erase variables created except the exit point
                    unsigned int variablesToRemove = igen.getStackSize() - endingScope.stackSize;
                    if(variablesToRemove > 0 && endingScope.identifier != "entry") //Only resize if there's things to remove and this isn't the program end
                    {
                        igen.genStackWalk(variablesToRemove);
                    }

                    //Insert the dynamic goto IF it's not the program entry point
                    if(endingScope.identifier != "entry")
                    {
                        igen.push(Variable("returnPoint", DataType::INT));
                        igen.genDynamicGoto();
                    }
                }
                else
                {
                    throw std::string("Ending unknown scope type");
                }

                //Store the scope in pastScopes and erase it from currentScopes as it's no longer open
                current.endPos = bytecode.size();
                if(openScopeStack.size() > 1 && openScopeStack[openScopeStack.size()-2].type == Scope::CLASS) //If scope was created inside of a class, store in that class's past scope list
                {
                    openScopeStack[openScopeStack.size()-2].childMemberScopes.emplace_back(*iter);
                }
                else //Else store in the global past scope list
                {
                    pastScopes.emplace_back(*iter);
                }
                iter = openScopeStack.erase(iter);
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
    //Reset the expected scope
    expectedScopeType.reset();

    //Extract arguments
    std::vector<std::string> arguments = parser.extractBracketArguments(line[1]);
    std::vector<std::vector<std::string>> initialisationArguments;

    //Store the bytecode size before the statement
    expectedScopeType.bytecodeSizeBefore = bytecode.size();

    //The for loop initialisation data must be parsed first and then passed to processVariable
    parser.tokenizeFile({arguments[0]}, initialisationArguments);
    unsigned int stackSizeBeforeInit = igen.getStackSize();
    processVariable(initialisationArguments[0]);
    expectedScopeType.initialisationVariableCount = igen.getStackSize() - stackSizeBeforeInit;

    //We set the statement position AFTER the loop initialisation, as we don't want the it to initialise every loop
    expectedScopeType.statementPos = bytecode.size();

    //Evaluate the condition of the loop
    evaluateBracket("(" + arguments[1] + ")");

    //Add the conditional IF instructions
    igen.genConditionalIf(0); //Skip pos will be set later once the scope end point is known

    //Set the expected scope data
    expectedScopeType.incrementor = "(" + arguments[2] + ")";
    expectedScopeType.type = Scope::FOR;
}

void Compiler::processElse(const std::vector<std::string>& line)
{
    //Ensure that previous scope was an else
    if(pastScopes.back().type != Scope::IF)
        throw std::string("Else must follow if");

    //Reset the expected scope
    expectedScopeType.reset();

    //Store the bytecode size before the statement
    expectedScopeType.bytecodeSizeBefore = bytecode.size();

    //Store type
    expectedScopeType.type = Scope::ELSE;
}

void Compiler::processWhile(const std::vector<std::string>& line)
{
    //Reset the expected scope
    expectedScopeType.reset();

    //Store the bytecode size before the statement
    expectedScopeType.bytecodeSizeBefore = bytecode.size();

    //Store the other stuff
    expectedScopeType.statementPos = bytecode.size();
    evaluateBracket(line[1]);
    igen.genConditionalIf(0); //Skip pos will be set later once the scope end point is known
    expectedScopeType.type = Scope::WHILE;
}

void Compiler::processIF(const std::vector<std::string>& line)
{
    //Reset the expected scope
    expectedScopeType.reset();

    //Store the bytecode size before the statement
    expectedScopeType.bytecodeSizeBefore = bytecode.size();

    //And the other stuff
    expectedScopeType.statementPos = bytecode.size();
    evaluateBracket(line[1]);
    igen.genConditionalIf(0); //Skip pos will be set later once the scope end point is known
    expectedScopeType.type = Scope::IF;
}

void Compiler::validateArgumentCount(unsigned int expected, unsigned int got)
{
    if(expected != got)
    {
        throw std::string("Malformed argument list, got " + std::to_string(got) + " when expected " + std::to_string(expected));
    }
}

void Compiler::processFunction(const std::vector<std::string>& line, bool destroyReturnValue)
{
    DataType possibleType = stringToDataType(line[0]);
    if(possibleType != DataType::UNKNOWN && line.size() > 2 && line[2][0] == '(') //If we're defining a function
    {
        //Reset the expected scope
        expectedScopeType.reset();

        //Add the function
        expectedScopeType.type = Scope::FUNCTION; //Store the scope type
        expectedScopeType.identifier = line[1]; //Store the function name
        expectedScopeType.incrementor = line[2]; //Store the function arguments
        expectedScopeType.returnType = possibleType; //Store the return value type

        //If this is the program entry point, update the entry point goto position
        if(line[1] == "entry")
            bytecode[1] = bytecode.size();
    }
    else //Else if we're calling a function
    {
        Scope *scope = getPastScope(line[0]);
        if(scope == nullptr)
            throw std::string("Failed to find entry point for '" + line[0] + "'");

        //Reserve space for return value if needed
        if(scope->returnType != DataType::VOID)
            igen.genCreateDefaultValue("", scope->returnType);

        //Create the exit point
        igen.genCreateInt("functionEnd", 0); //Add a bit for the stack walk below
        unsigned int exitPointPos = bytecode.size()-1;

        //Evaluate arguments, adding to the stack
        unsigned int sizeBeforeEvaluation = igen.getStackSize();
        evaluateBracket(line[1]);
        unsigned int sizeAfterEvaluation = igen.getStackSize();

        //Artificially remove them from the compiler's stack, as the values are already cleaned up by the function scope end
        for(unsigned int a = sizeBeforeEvaluation; a < sizeAfterEvaluation; a++)
            igen.pop();

        //update the exit point position
        bytecode[exitPointPos] = bytecode.size()+2;

        //Goto the function
        igen.genGoto(scope->startPos);

        //Remove exit point, it will have been used by the function
        igen.pop();

        //Remove return value if specified
        if(scope->returnType != DataType::VOID && destroyReturnValue)
        {
            igen.genStackWalk(1);
        }
    }
}

void Compiler::processVariable(const std::vector<std::string>& line) //things like "int a = 20"
{
    DataType possibleType = stringToDataType(line[0]);
    if(possibleType == DataType::UNKNOWN && line[0] != "auto") //If we're not creating a new variable (eg a = 20)
    {
        if(line[1] == "=") //If it's a set operation
        {
            //Evaluate the bracket containing what the variable should be set to
            evaluateBracket("(" + parser.combineArguments(line, 2, line.size() - 2) + ")");
        }
        else //Else if it's a math operation
        {
            //First move the variable data that we're operating on to the top of the stack
            igen.genCloneTop(line[0]);

            //Evaluate the bracket containing what the variable should be set to
            evaluateBracket("(" + parser.combineArguments(line, 2, line.size() - 2) + ")");

            //Do different things depending on the assignment operator
            if(line[1] == "*=")
            {
                igen.genMathMultiply(2);
            }
            else if(line[1] == "/=")
            {
                igen.genMathDivide(2);
            }
            else if(line[1] == "%=")
            {
                //Divide the last two things on the stack (the bracket and the variable)
                igen.genMathModulus(2);
            }
            else if(line[1] == "+=")
            {
                igen.genMathAdd(2);
            }
            else if(line[1] == "-=")
            {
                igen.genMathSubtract(2);
            }
            else if(line[1] == "@=")
            {
                igen.genConcentrateStrings(2);
            }
        }

        //Set the variable to the last thing on the stack
        igen.genSetVariable(line[0]);
    }
    else //If we're creating a new variable (eg int a = 20)
    {
        //Convert its data type from string to enum
        DataType type = stringToDataType(line[0]);

        if(!openScopeStack.empty() && openScopeStack.back().type == Scope::CLASS) //We're defining class members, so don't actually create them. Just register to the class.
        {
            std::cout << "\nVar in class: " << line[1];
            //NOTE TO SELF: ADD BRACKET INITIALISATION EVALUATION
            pastScopes.back().childDataMembers.emplace_back(Variable(line[1], type));
        }
        else //If we're not in a class, create the variables immediately
        {
            if(line.size() > 2) //If there's a value provided (int a = 20)
            {
                //Evaluate value to set it to and name the variable
                evaluateBracket("(" + parser.combineArguments(line, 3, line.size() - 3) + ")");
                igen.renameVariable(igen.getStackSize()-1, line[1]);
            }
            else //Else no default value provided (int a)
            {
                if(parser.isArrayDefinition(line[1])) //Check if it's an array definition
                {
                    //Split array definition into name and size
                    std::string arrayName;
                    std::string arraySize;
                    parser.splitArrayDefinition(line[1], arrayName, arraySize);

                    //Create the array
                    igen.genCreateArray(arrayName, arraySize, possibleType);
                }
                else
                {
                    igen.genCreateDefaultValue(line[1], possibleType);
                }
            }
        }
    }
    return;
}

void Compiler::processConsole(const std::vector<std::string>& line)
{
    std::string operation = line[1];
    std::string data = line[2];
    if(operation == "print")
    {
        //Store old stack size for calculating how many things have been added to the stack during bracket evaluation
        auto stackSizeOld = igen.getStackSize();
        evaluateBracket(data);
        igen.genConsoleOut(igen.getStackSize() - stackSizeOld);

    }
    else if(operation == "scan")
    {
        std::vector<std::string> bracket;
        parser.extractBracket(data, bracket);
        igen.genConsoleIn(bracket[0]);
    }
    else
    {
        throw std::string("Unknown Console operation: " + operation);
    }
}

unsigned int Compiler::evaluateBracket(std::string originalLine)
{
    unsigned int variablesOnStack = 0;
    variablesOnStack = 0;
    auto addVariableToStack = [&] (const std::string &data) -> void
    {
        if(igen.isVariable(data) != -1) //If it's a variable
        {
            igen.genCloneTop(data);
        }
        else //Else if it's raw data
        {
            if(data[0] == '"') //If string
            {
                igen.genCreateString("", data.substr(1, data.size()-2));
            }
            else if(data[0] == '\'') //If character
            {
                igen.genCreateChar("", data[1]);
            }
            else if(data == "true") //If boolean
            {
                igen.genCreateBool("", true);
            }
            else if(data == "false") //If boolean
            {
                igen.genCreateBool("", false);
            }
            else //Else if number
            {
                igen.genCreateInt("", stoull(data));
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
            if(segments.size() == 1)
            {
                igen.genOperator(segmentInstruction, variablesOnStack);
                variablesOnStack = 0;
            }
            else
            {
                igen.genOperator(segmentInstruction, segments.size() - 1);
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

    //Bracket all arguments, removing unneeded spaces
    for(auto &arg : arguments)
    {
        while(arg[0] == ' ')
            arg.erase(0, 1);
        arg.insert(0, "(");
        while(arg[arg.size()-1] == ' ')
            arg.erase(arg.size()-1, 1);
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
        for(unsigned int a = 0; a < components.size(); a++)
        {
            const std::string &segment = components[a];

            //If it's a function, call it
            Scope *possibleFunction = getPastScope(segment);
            if(possibleFunction != nullptr) //It's a function
            {
                //Take arguments from next segment only if the function accepts arguments
                std::string arg;
                if(possibleFunction->argumentCount > 0)
                    arg = components[++a];

                //Call it with the provided arguments if any, not destroying the return value
                processFunction({segment, arg}, false);
                variablesOnStack++;
                a++;
            }
            else //It's not a function
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
    }
    return 0;
}

void Compiler::displayWarning(const std::string& message)
{
    std::cout << "\nWarning on line " << line << ": " << message << std::endl;
}

Scope *Compiler::getPastScope(const std::string& identifier)
{
    //Find the function's starting point from lastScopes
    Scope *scope = nullptr; //Scope we're calling
    auto scopeIter = std::find_if(pastScopes.rbegin(), pastScopes.rend(), [&] (const Scope &next) {if(next.identifier == identifier){return true;}return false;});
    if(scopeIter != pastScopes.rend()) //If it was found
    {
        scope = &(*scopeIter); //Set found scope pointer to the correct past scope
    }
    if(scope == nullptr && identifier == expectedScopeType.identifier) //If we're recursively calling this scope
    {
        scope = &expectedScopeType;
    }
    return scope;
}



