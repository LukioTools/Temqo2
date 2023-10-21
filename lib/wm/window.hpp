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

    struct Pos{
        u_short x = 0;
        u_short y = 0;
    };

        //when a resieze events are called, shall it do refresh
    class Window
    {
    private:
        Pos pos = {0,0};
        u_short width = 0;
        u_short height = 0;
        DisplayMode mode = DisplayMode::UNDEFINED;
    public:
        void refresh(Pos p, u_char h){
            pos = p;
            height = h;
        }
        Window & operator=(const Window&) = delete;
        Window(const Window&) = delete;

        Window(DisplayMode m, Pos p, u_char h, u_char w): mode(m), pos(p), height(h), width(w) {}
        Window(DisplayMode m, u_short x, u_short y, u_char h, u_char w): mode(m), pos({.x=x, .y=y}), height(h), width(w) {}
        ~Window() {}
    };
} // namespace wm
#endif // C_WINDOWS
