#include <sys/types.h>
#include "globals.hpp"
#if !defined(C_WINDOWS)
#define C_WINDOWS

namespace wm
{

    
    enum DisplayMode : u_char{   
        UNDEFINED,
        ABSOLUTE,
        RELATIVE,
        PERCENT,
    };



        //Simple position holder
    struct Window
    {
        u_short x = 0;
        u_short y = 0;
        u_short w = 0; 
        u_short h = 0;

        u_char padding_updown = 0;
        u_char padding_sides = 0;


        Window & operator=(const Window&) = delete;
        Window(const Window&) = delete;

        Window(u_short _x, u_short _y, u_short _w, u_short _h): x(_x), y(_y), h(_h), w(_w) {}
        ~Window() {}
    };
} // namespace wm

 
#endif // C_WINDOWS
