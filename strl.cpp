#include "custom/stringlite.hpp"
#include <iostream>


int main(int argc, char const *argv[])
{
    StringLite str = "Hello World!";
    
    std::cout << str << std::endl;

    StringLite str2 = str;

    std::cout << str2 << std::endl;

    return 0;
}
