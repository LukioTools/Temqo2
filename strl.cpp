#include "custom/stringlite.hpp"
#include <iostream>

StringLite str = "Hello World!";

int main(int argc, char const *argv[])
{
    std::cout << str << std::endl;


    StringLite str2 = str.get_p();

    std::cout << str2 << std::endl;

    return 0;
}
