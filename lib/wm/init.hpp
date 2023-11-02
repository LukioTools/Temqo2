#pragma once
#include <iostream>
#include <termios.h>
#include "def.hpp"
#include "resize.hpp"

namespace wm
{
    
    inline int init(){
        
        //std::locale::global(std::locale("en_US.UTF-8"));

        use_attr(alt_buffer << enable_mouse(USE_MOUSE));
        signal(SIGWINCH, wm::resize);
        clear_all();
        wm::resize(SIGWINCH);



        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        return 0;
    }

    inline int deinit(){
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        use_attr(disable_mouse(USE_MOUSE) << norm_buffer << cursor_visible);
        return 0;
    }
} // namespace wm
