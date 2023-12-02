#include "custom/stringlite.hpp"
#include <iostream>
#include <iterator>
#include <vector>


int main(int argc, char const *argv[])
{
    StringLite str = "Hello World!";
    
    std::cout << str << std::endl;

    StringLite str2 = str;

    std::cout << str2 << std::endl;
 
    for (auto ch : str2)
    {
        std::cout << ch << std::endl;
    }
    
    return 0;
}
