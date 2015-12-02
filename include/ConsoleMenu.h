#ifndef CONSOLEMENU_H
#define CONSOLEMENU_H

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <limits>

class ConsoleMenu
{
    public:
        /** Default constructor */
        ConsoleMenu(const std::string &menuTitle);
        /** Default destructor */
        virtual ~ConsoleMenu();

        void addOption(const std::string &text, std::function<void()> callback);

        void display();
    protected:

    private:
        std::string title;
        std::vector<std::pair<std::string, std::function<void()>>> options;
};

#endif // CONSOLEMENU_H
