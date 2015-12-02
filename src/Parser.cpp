#include "Parser.h"

Parser::Parser()
{
    //ctor
}

Parser::~Parser()
{
    //dtor
}

bool Parser::parseFile(const std::string &filepath, std::vector<std::string> &data)
{
    //Open the source file
    std::ifstream file(filepath);
    if(!file.is_open())
        return false;

    //Iterate through each line and store data
    std::string lineBuffer;
    unsigned int bracketDepth = 0;
    bool isBracketOpen = false;
    while(std::getline(file, lineBuffer))
    {
        std::string wordBuffer;
        for(const auto &c : lineBuffer)
        {
            if((c == ' ' || c == '.') && isBracketOpen == false)
            {
                data.emplace_back(wordBuffer);
                wordBuffer.clear();
            }
            else
            {
                wordBuffer += c;
            }

            if(c == '(')
            {
                isBracketOpen = true;
                bracketDepth++;
            }
            else if(c == ')')
            {
                bracketDepth--;
                if(bracketDepth == 0)
                    isBracketOpen = false;
            }
        }
        data.emplace_back(wordBuffer);
    }
    return true;
}
