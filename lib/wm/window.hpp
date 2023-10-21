#include <sys/types.h>
#if !defined(C_WINDOWS)
#define C_WINDOWS


namespace wm
{
    unsigned int WIDTH = 0, HEIGHT = 0;
    enum DisplayMode : u_char{   
        UNDEFINED,
        ABSOLUTE,
        RELATIVE,
        PERCENT,
    };


    class Window
    {
    private:
        DisplayMode mode = ABSOLUTE;
        u_char width = 0;
    public:
        Window(DisplayMode m, u_char w): mode(m), width(w) {}
        ~Window() {}
    };
} // namespace wm
#endif // C_WINDOWS
