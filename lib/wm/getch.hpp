#pragma once

#include <iostream>

#include "def.hpp"
namespace wm
{
    

    inline int getch(){

        int ch = std::cin.get();
        if(ch != '\x1b'){//escape
            return ch;
        }

        int c = std::cin.get();
        ch+=c<<8;

        if(c !='\x5b'){ //cursor 0x004x5b1b x = 1-4 (up, down, M_LEFT, M_RIGHT)
            return ch;
        }

        c = std::cin.get();
        
        if(c != '\x4d'){//mouse event
            ch+=c<<16;
            return ch;
        }

        unsigned int btn = 0, x = 0, y = 0;
        btn = std::cin.get();
        x = std::cin.get()-'\x21';
        y = std::cin.get()-'\x21';
        ch = '\xFF' + (btn<<8) + (x << 16) + (y << 24);
                
        return ch;
    }


    enum KEY : u_char{
        K_UNDEFINED = 0,
        K_UP, 
        K_DOWN, 
        K_LEFT,
        K_RIGHT, 
    };



    inline std::string to_str(KEY key){
        switch (key)
        {
        CASE_STR(K_UNDEFINED);
        CASE_STR(K_UP);
        CASE_STR(K_DOWN);
        CASE_STR(K_LEFT);
        CASE_STR(K_RIGHT);
        default:
            return "UNKNOWN";
        }
    }
    
    inline wm::KEY is_key(int input) noexcept{
    auto ptr = reinterpret_cast<char*>(&input);
    if(ptr[0] != '\x1b' || ptr[1] != '\x5b'|| ptr[2] > '\x44' || ptr[2] < '\x41'){
        return wm::K_UNDEFINED;
    }
    switch (ptr[2])
    {
    case '\x41':    return wm::K_UP;
    case '\x42':    return wm::K_DOWN;
    case '\x43':    return wm::K_RIGHT;
    case '\x44':    return wm::K_LEFT;
    default:        return wm::K_UNDEFINED;
    }
}

} // namespace wm
