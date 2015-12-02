#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

class Parser
{
    public:
        /** Default constructor */
        Parser();
        /** Default destructor */
        virtual ~Parser();

        bool parseFile(const std::string &filepath, std::vector<std::string> &data);
    protected:
    private:
};

#endif // PARSER_H
