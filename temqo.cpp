#include "temqo.hpp"
#include "lib/wm/getch.hpp"
#include <unistd.h>





int main(int argc, char** const argv)
{
    temqo::init(argc, argv);


    while (auto ch = wm::getch())
    {
        if(ch == 'q')
            break;
    }
    
    //temqo::deinit();

    return 0;
}
