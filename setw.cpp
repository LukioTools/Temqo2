

#include "lib/wm/getch.hpp"
#include <iomanip>
#include <iostream>
int main(int argc, char const *argv[])
{
    while (true) {
        auto ch = wm::getch();
        std::cout << ch << std::endl;
    }

    return 0;
}
