#include "Parser.h"

Parser::Parser()
{
    //ctor
    seperatorTokens = {' ', '.', ',', '"', '('};
}

Parser::~Parser()
{
    //dtor
}

void Parser::tokenizeFile(const std::vector<std::string>& data_in, std::vector<std::vector<std::string>>& data_out)
{
    for(const auto &line : data_in) //Iterate through each line
    {
        data_out.emplace_back(std::vector<std::string>());
        std::vector<std::string> &currentLine = data_out.back();
        std::string lineBuffer = "";
        bool isQuotationOpen = false;
        bool isBracketOpen = false;
        unsigned int bracketDepth = 0;
        for(const auto &c : line) //Iterate through each character in each line
        {
            bool isSeperator = false;
            for(unsigned int a = 0; a < seperatorTokens.size(); a++) //Check to see if it's a split token
            {
                if(c == '"')
                {
                    isQuotationOpen = !isQuotationOpen;
                    isBracketOpen = true;
                }
                else if(c == '(')
                {
                    bracketDepth++;
                    isBracketOpen = true;
                }
                else if(c == ')')
                {
                    bracketDepth--;
                    if(bracketDepth == 0)
                        isBracketOpen = false;
                }
                if(!isQuotationOpen && c == seperatorTokens[a] && !isBracketOpen) //If it is, store the buffered word
                {
                    if(!lineBuffer.empty())
                        currentLine.emplace_back(lineBuffer);
                    lineBuffer.clear();
                    isSeperator = true;
                    a = seperatorTokens.size();
                }
                else if(isBracketOpen && c == '(' && bracketDepth == 1)
                {
                    if(!lineBuffer.empty())
                        currentLine.emplace_back(lineBuffer);
                    lineBuffer.clear();
                    a = seperatorTokens.size();
                }
            }
            if(!isSeperator || isQuotationOpen) //If not a separator, add to buffered word
            {
                lineBuffer += c;
            }
        }
        if(!lineBuffer.empty())
            currentLine.emplace_back(lineBuffer);
    }

    //Process escape sequences
    for(auto &a : data_out)
    {
        for(auto &b : a)
        {
            processEscapeSequences(b);
        }
    }
}

std::string Parser::bracketOperatorFix(const std::string &data)
{
    //Convert things like "((1 + 2) + (3 * 4))" to "((1 2 +) (3 4 *) +)"

    std::string returnValue; //Store what the function will return as it's generated
    unsigned int currentLayer = 0; //To keep track of the current bracket layer
    std::map<unsigned int, std::string> bracketOperators; //Keep track of each bracket layer's operator. Map to keep track of multiple layers of brackets. map<LAYER, OPERATOR>
    bool isQuoteOpen = false;

    //Find operators and data
    for(unsigned int a = 0; a < data.size(); a++)
    {
        //If quotation marks
        if(data[a] == '"')
        {
            isQuoteOpen = !isQuoteOpen;
        }

        //If new bracket, update bracket layer and set the layer's default operator (+)
        if(!isQuoteOpen && data[a] == '(')
        {
            currentLayer++;
            bracketOperators[currentLayer] = "";
            returnValue += data[a];
        }

        //If we're exiting a bracket, update the layer and insert a space followed by this layer's operator
        else if(!isQuoteOpen && data[a] == ')')
        {
            if(!bracketOperators[currentLayer].empty()) //Only add operator to the end if there is one
            {
                returnValue += ' ';
                returnValue += bracketOperators[currentLayer];
            }
            returnValue += data[a];
            currentLayer--;
        }

        //Else if dual-character operator. Temporary fix.
        else if(!isQuoteOpen && a != data.size() && stringToInstruction(data.substr(a, 2)) != -1)
        {
            bracketOperators[currentLayer] = data.substr(a, 2);
            returnValue.erase(returnValue.size()-1, 1);
            a++;
        }

        //Else if a single-character operator, don't add the operator to the return value and store this layer's operator
        else if(!isQuoteOpen && stringToInstruction(data.substr(a, 1)) != -1)
        {
            bracketOperators[currentLayer] = data.substr(a, 1);
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

void Parser::extractBracket(const std::string &bracket, std::vector<std::string> &results)
{
    //If there's nothing in the bracket, nothing to do
    if(bracket.empty())
        return;

    //Initialise variables
    int bracketStartPos = -1;
    unsigned int bracketStartLayer = -1;
    unsigned int bracketDepth = 0;
    std::string bracketBuffer;
    std::string bracketOperator;
    bool isQuoteOpen = false;
    std::string preBracketInfo;

    //Iterate through each character in the vector and process it
    for(unsigned int a = 0; a < bracket.size(); a++)
    {
        //Keep track of if quotes are open
        if(bracket[a] == '"')
            isQuoteOpen = !isQuoteOpen;

        //If a bracket is open, add the data to the bracket buffer
        if(bracketStartPos != -1)
            bracketBuffer += bracket[a];


        //Else if dual-character operator. Temporary fix.
        else if(!isQuoteOpen && a != bracket.size() && stringToInstruction(bracket.substr(a, 2)) != -1)
        {
            bracketOperator = bracket.substr(a, 2);
            a++;
        }

        //Else if a single-character operator, don't add the operator to the return value and store this layer's operator
        else if(!isQuoteOpen && stringToInstruction(bracket.substr(a, 1)) != -1)
        {
            bracketOperator = bracket.substr(a, 1);
        }
        else if(bracket[a] != '(' && bracket[a] != ')')
        {
            preBracketInfo += bracket[a];
        }

        //If quotes are not open do these checks
        if(!isQuoteOpen)
        {
            if(bracket[a] == '(') //If bracket open, increase bracket layer
            {
                if(bracketStartPos == -1) //If no bracket open, set current bracket variables
                {
                    bracketStartLayer = bracketDepth;
                    bracketStartPos = a;

                    if(!preBracketInfo.empty())
                    {
                        while(preBracketInfo[0] == ' ')
                            preBracketInfo.erase(0, 1);
                    }
                    if(!preBracketInfo.empty())
                    {
                        while(preBracketInfo[preBracketInfo.size()-1] == ' ')
                            preBracketInfo.erase(preBracketInfo.size()-1, 1);
                    }
                    if(!preBracketInfo.empty())
                    {
                        results.emplace_back(preBracketInfo);
                        preBracketInfo.clear();
                    }
                }
                bracketDepth++;
            }
            else if(bracket[a] == ')') //If bracket close, decrease bracket depth and process the closed bracket
            {
                bracketDepth--;
                if(bracketDepth == bracketStartLayer) //If the current bracket is ending
                {
                    if(bracketBuffer.find("(") == std::string::npos) //If there's NOT a sub bracket
                    {
                        if(!bracketBuffer.empty()) //And the bracket buffer isn't empty
                        {
                            bracketBuffer.erase(bracketBuffer.size()-1, 1); //Remove the ending ')'
                            results.emplace_back(bracketBuffer); //Then store the bracket
                        }
                    }
                    else //Else it contains a sub-bracket so recursively extract it
                    {
                        bracketBuffer.erase(bracketBuffer.size()-1, 1);
                        extractBracket(bracketBuffer, results);
                    }

                    //Reset current bracket variables for parsing the next one
                    bracketStartLayer = -1;
                    bracketStartPos = -1;
                    bracketBuffer.clear();
                }
            }
        }
    }

    //If the bracket contained an operator, add it back to the end
    if(!bracketOperator.empty())
        results.emplace_back(bracketOperator);
}

void Parser::processEscapeSequences(std::string& data)
{
    replaceAll(data, "\\n", "\n"); //New line
    replaceAll(data, "\\t", "\t"); //Horizontal tab
    replaceAll(data, "\\v", "\v"); //Vertical tab
    replaceAll(data, "\\b", "\b"); //Backspace
    replaceAll(data, "\\r", "\r"); //Carriage return
    replaceAll(data, "\\f", "\f"); //Form feed
    replaceAll(data, "\\a", "\a"); //Alert
}

void Parser::replaceAll(std::string& source, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while((pos = source.find(from, pos)) != std::string::npos)
    {
        source.replace(pos, from.size(), to);
        pos += to.size();
    }
}

std::vector<std::string> Parser::extractBracketArguments(std::string data)
{
    std::vector<std::string> arguments;
    std::string argumentBuffer;
    bool isQuoteOpen = false;
    int bracketDepth = 0;

    //Take off surrounding brackets if any as these aren't needed in the split results
    if(data[0] == '(')
        data.erase(0,1);
    if(data[data.size()-1] == ')')
        data.erase(data.size()-1, 1);

    //Go through each character and split each argument into a separate vector element
    for(unsigned int c = 0; c < data.size(); c++) //Ignore opening and close brackets
    {
        if(data[c] == '"')
            isQuoteOpen = !isQuoteOpen;
        if(data[c] == '(')
            bracketDepth++;
        else if(data[c] == ')')
            bracketDepth--;
        if(data[c] == ',' && !argumentBuffer.empty() && !isQuoteOpen && bracketDepth == 0)
        {
            arguments.emplace_back(argumentBuffer);
            argumentBuffer.clear();
        }
        else
        {
            argumentBuffer += data[c];
        }
    }
    if(!argumentBuffer.empty())
        arguments.emplace_back(argumentBuffer);

    //Fix the spacing
    for(auto &arg : arguments)
    {
        while(arg[0] == ' ')
            arg.erase(0,1);
        while(arg[arg.size()-1] == ' ')
            arg.erase(arg.size()-1, 1);
    }

    return arguments;
}

std::string Parser::combineArguments(const std::vector<std::string>& args, unsigned int first, unsigned int count)
{
    std::string rv;
    for(unsigned int a = 0; a < count; a++)
        rv += (args[first + a] + " ");
    rv.erase(rv.size()-1, 1);
    return rv;
}
