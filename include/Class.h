#ifndef CLASS_H
#define CLASS_H
#include <string>
#include <vector>

#include "VMTypes.h"


class Class : public Variable
{
    public:
        /** Default constructor */
        Class(const std::string &name);
        /** Default destructor */
        virtual ~Class();
    protected:
    private:
        std::vector<Scope> childMemberFunctions;
        std::vector<Variable> childDataMembers;
};

#endif // CLASS_H
