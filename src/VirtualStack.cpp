#include "VirtualStack.h"

VirtualStack::VirtualStack()
{
    //ctor
}

VirtualStack::~VirtualStack()
{
    //dtor
}

void VirtualStack::push(const Variable& var)
{
    stack.emplace_back(var);
}

Variable VirtualStack::pop()
{
    Variable rv = stack.back();
    stack.pop_back();
    return rv;
}

int VirtualStack::isVariable(const std::string& identifier)
{
    for(unsigned int a = 0; a < stack.size(); a++)
    {
        if(stack[a].identifier == identifier)
            return a; //Found, return stack position
    }
    return -1; //Not found
}

unsigned int VirtualStack::getStackSize()
{
    return stack.size();
}

void VirtualStack::resize(unsigned int newSize)
{
    stack.erase(stack.begin() + newSize, stack.end());
}

const Variable &VirtualStack::getVariable(unsigned int stackPos)
{
    return stack[stackPos];
}

void VirtualStack::renameVariable(unsigned int stackPos, const std::string &identifier)
{
    stack[stackPos].identifier = identifier;
}
