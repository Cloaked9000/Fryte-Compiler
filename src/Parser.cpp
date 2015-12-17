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

void Parser::tokenizeFile(std::vector<std::string>& data_in, std::vector<std::vector<std::string>>& data_out)
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

void Parser::extractBracket(std::string bracket, std::vector<std::string> &results)
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
