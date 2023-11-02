#include "lib/wm/def.hpp"
#include <iostream>
int main(int argc, char const *argv[])
{
    std::cout << enable_mouse(SET_X10_MOUSE) << std::flush; // en tiiä tarviiks flush


    //loop where shit
    while (true) {
    
    }


    std::cout << disable_mouse(SET_X10_MOUSE) << std::flush; // en tiiä tarviiks flush

    return 0;
}
