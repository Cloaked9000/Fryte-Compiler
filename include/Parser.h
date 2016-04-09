#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

#include "VMTypes.h"

class Parser
{
    public:
        Parser();
        virtual ~Parser();
        void tokenizeFile(const std::vector<std::string> &data_in, std::vector<std::vector<std::string>> &data_out);
        std::string bracketOperatorFix(const std::string &data); //Fix bracket operators, things like (1 + 1) to (1 1 +)
        void extractBracket(const std::string &bracket, std::vector<std::string> &results); //Extract all brackets in a line and move into a vector
        void processEscapeSequences(std::string &data); //Replace things like '\n' with the newline character
        std::vector<std::string> extractBracketArguments(std::string data); //Extracts comma separated arguments out of a bracket
        std::string combineArguments(const std::vector<std::string> &args, unsigned int first, unsigned int count); //As arguments can be split over several vector elements, they need to be put together before being evaluated
    protected:
    private:
        void replaceAll(std::string &data, const std::string &from, const std::string &to); //Replaces all instances of a string with another
        std::vector<unsigned char> seperatorTokens;
};

#endif // PARSER_H
