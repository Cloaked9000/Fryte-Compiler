#include <iostream>
#include <string>
#include <limits>
#include <map>

using namespace std;

#include "ConsoleMenu.h"
#include "BytecodeIO.h"
#include "Parser.h"
#include "Compiler.h"

int main()
{
    Parser parser;
    Compiler compiler;

    std::vector<std::string> sourceData;
    std::vector<unsigned char> compiledBytecode;

    parser.parseFile("doIt.txt", sourceData);

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

    BytecodeIO::writeBytecode("out.fry", &compiledBytecode);
    return 0;
}
