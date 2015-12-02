#include "BytecodeIO.h"

BytecodeIO::BytecodeIO()
{
    //ctor
}

BytecodeIO::~BytecodeIO()
{
    //dtor
}

bool BytecodeIO::readBytecode(const std::string& filepath, std::vector<unsigned char>* byteCode)
{
    std::streampos fileSize;
    char * memblock;

    std::ifstream file;
    file.open(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return false;

    fileSize = file.tellg();
    memblock = new char [fileSize];
    file.seekg(0, std::ios::beg);
    file.read(memblock, fileSize);
    file.close();

    for(unsigned int a = 0; a < fileSize; a++)
        byteCode->emplace_back(memblock[a]);

    delete[] memblock;
    return true;
}

bool BytecodeIO::writeBytecode(const std::string& filepath, const std::vector<unsigned char>* byteCode)
{
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::binary);
    if(!file.is_open())
        return false;

    for(const auto &byte: *byteCode)
        file << byte;

    for(const auto &byte: *byteCode)
        std::cout << (int)byte << ", ";

    file.close();
    return true;
}
