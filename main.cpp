#include <iostream>
#include <string>
#include <limits>
#include <map>

using namespace std;

#include "BytecodeIO.h"
#include "Parser.h"
#include "Compiler.h"

int main(int argc, char *argv[])
{
    if(argc < 3)
        return -1;

    Parser parser;
    Compiler compiler;

    std::vector<std::string> sourceData;
    std::vector<unsigned char> compiledBytecode;
    std::cout << "\nParsing... ";
    parser.parseFile(argv[1], sourceData);
    std::cout << "\nCompiling... ";
    try
    {
        compiledBytecode = compiler.compileSource(sourceData);
    }
    catch(const std::string &e)
    {
        std::cout << "\nError: " << e;
    }
    catch(const std::exception &e)
    {
        std::cout << "\nException: " << e.what();
    }

    BytecodeIO::writeBytecode(argv[2], &compiledBytecode);
    return 0;
}
