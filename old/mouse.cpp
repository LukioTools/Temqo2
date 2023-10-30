

#include <iostream>
#include <ostream>
int main(int argc, char const *argv[])
{
    std::cout << "\e[?1007h" << std::flush;

    getchar();
    return 0;
}
