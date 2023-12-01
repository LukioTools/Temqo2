#include "temqo.hpp"
#include "lib/wm/getch.hpp"
#include <unistd.h>





int main(int argc, char** const argv)
{
    temqo::clog << "Starting\n";
    temqo::init(argc, argv);
    temqo::clog << "Initialized\n";


    while (auto ch = wm::getch())
    {
        if(ch == 'q')
            break;
    }
    
    //temqo::deinit();
    try{
        temqo::p.save();
    }catch(...){}

    return 0;
}
