#pragma once
#include <atomic>
#include <termios.h>


namespace wm
{
    struct termios oldt, newt;
    unsigned int WIDTH = 0, HEIGHT = 0;
    std::atomic_bool resize_event;
    
} // namespace wm

#define aspect_ratio static_cast<double>(wm::WIDTH)/static_cast<double>(wm::HEIGHT)


