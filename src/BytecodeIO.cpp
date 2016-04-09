#include "BytecodeIO.h"

BytecodeIO::BytecodeIO()
{
    //ctor
}

BytecodeIO::~BytecodeIO()
{
    //dtor
}

bool BytecodeIO::readBytecode(const std::string& filepath, std::vector<unsigned int>& bytecode)
{
    //Open the file
    std::ifstream file;
    file.open(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return false;

    //Get file size
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    //Reserve space in the vector and read the data
    bytecode.resize(fileSize / sizeof(decltype(bytecode.begin())::value_type));
    file.read((char*)&bytecode[0], fileSize);

    file.close();
    return true;
}

bool BytecodeIO::writeBytecode(const std::string& filepath, const std::vector<unsigned int>& bytecode)
{
    std::ofstream file;
    file.open(filepath, std::ios::out | std::ios::binary);
    if(!file.is_open())
        return false;

    file.write((char*)bytecode.data(), sizeof(decltype(bytecode.begin())::value_type) * bytecode.size());

    std::cout << "\nBytecode: ";
    for(const auto &byte: bytecode)
        std::cout << (unsigned int)byte << ", ";
    std::cout << std::endl;
    file.close();
    return true;
}

bool BytecodeIO::readFile(const std::string& filepath, std::vector<std::string> &data)
{
    std::ifstream file(filepath);
    if(!file.is_open())
        return false;

    std::string buffer;
    while(std::getline(file, buffer))
        data.emplace_back(buffer);

    file.close();
    return true;
}
