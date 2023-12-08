

#include <iomanip>
#include <iostream>
int main(int argc, char const *argv[])
{
    std::cout << std::setfill('0') << std::setw(7) << "Hello world"<< std::endl;
    return 0;
}
