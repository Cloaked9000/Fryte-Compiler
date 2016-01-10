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

        unsigned int getStackSize();
        Variable getVariable(unsigned int stackPos);

        int isVariable(const std::string &identifier);
        void renameVariable(unsigned int stackPos, const std::string &identifier);

        void resize(unsigned int newSize);
        void push(const Variable &var);
        Variable pop();
    protected:
    private:
        const static int maxStackSize = 500;
        Variable stack[maxStackSize];
        unsigned int stackOffset;
};

#endif // VIRTUALSTACK_H
