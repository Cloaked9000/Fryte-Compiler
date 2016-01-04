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

unsigned int VirtualStack::size()
{
    return stack.size();
}

unsigned int VirtualStack::resize(unsigned int newSize)
{
    stack.erase(stack.begin() + (newSize - stack.size() + 1), stack.end());
}
