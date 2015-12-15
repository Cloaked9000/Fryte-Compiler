#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "VMTypes.h"

class Parser
{
    public:
        Parser();
        virtual ~Parser();
        void tokenizeFile(std::vector<std::string> &data_in, std::vector<std::vector<std::string>> &data_out);
        std::string bracketOperatorFix(const std::string &data);
        void bracketOperatorFixNew(const std::string &data, std::string &result);
        void extractBracket(std::string bracket, std::vector<std::string> &results);
    protected:
    private:
        std::vector<unsigned char> seperatorTokens;
};

#endif // PARSER_H
