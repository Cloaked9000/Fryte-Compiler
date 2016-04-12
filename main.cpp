#include <iostream>
#include <vector>
#include <string>
#include <cctype>

#include "Parser.h"
#include "BytecodeIO.h"
#include "Compiler.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cout << "\nExpected 2 arguments, got " << argc << ". -source -destination" << std::endl;
        return -1;
    }

    Parser parser;
    std::vector<unsigned int> output;
    std::vector<std::string> data;
    if(!BytecodeIO::readFile(argv[1], data))
        return -1;

    //return 0;
    Compiler compiler;
    compiler.compile(data, output);

    //Save to file
    BytecodeIO::writeBytecode(argv[2], output);
    return 0;
}





