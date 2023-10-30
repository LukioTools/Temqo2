

#include <cstdlib>
#include <cstring>
#include <iostream>



int main(int argc, char const *argv[])
{
    std::locale::global(std::locale("en_US.UTF-8"));
    
    constexpr size_t n = 9;
    constexpr size_t size = sizeof(char16_t)*n;


    return 0;
}
