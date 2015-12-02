#include "ConsoleMenu.h"

ConsoleMenu::ConsoleMenu(const std::string &name)
{
    //ctor
    title = name;
}

ConsoleMenu::~ConsoleMenu()
{
    //dtor
}

void ConsoleMenu::addOption(const std::string& text, std::function<void()>callback)
{
    options.emplace_back(std::make_pair(text, callback));
}

void ConsoleMenu::display()
{
    std::cout << title;
    for(unsigned int a = 0; a < options.size(); a++)
        std::cout << "\n[" << a << "]: " << options[a].first;

    std::cout << "\nOption: ";
    unsigned int option;
    std::cin >> option;

    options[option].second();
}
