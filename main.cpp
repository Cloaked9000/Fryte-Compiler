#include <iostream>
#include <vector>
#include <string>

#include "Parser.h"
#include "BytecodeIO.h"
#include "Compiler.h"

using namespace std;

int main()
{
    std::vector<std::string> data;
    if(!BytecodeIO::readFile("file.txt", data))
        return -1;

    //return 0;
    Compiler compiler;
    compiler.compile(data);
    return 0;
}
