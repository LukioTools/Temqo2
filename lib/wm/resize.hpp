#pragma once


#include <signal.h>
#include <sys/ioctl.h>
#include <atomic>
#include "globals.hpp"

namespace wm
{


    inline void resize(int sig){
        if(sig != SIGWINCH) {return;}
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        WIDTH = w.ws_col-1;
        HEIGHT = w.ws_row-1;
        resize_event = true;
    }
    
} // namespace wm
