#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H

#include <vector>
#include <string>
#include <iostream>

#include "VMTypes.h"

class VirtualStack
{
    public:
        VirtualStack();
        virtual ~VirtualStack();

        void push(const Variable &var);
        Variable pop();
        unsigned int size();
        unsigned int resize(unsigned int newSize);

        int isVariable(const std::string &identifier);
    protected:
    private:
        std::vector<Variable> stack;
};

#endif // VIRTUALSTACK_H
