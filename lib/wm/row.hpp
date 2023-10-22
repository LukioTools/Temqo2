#include <cmath>
#include <sys/types.h>
#include <vector>
#if !defined(C_ROW)
#define C_ROW
#include "window.hpp"
#include "globals.hpp"

namespace wm
{
    //all absolute values
    unsigned int row_absolute = 0;
    //all percent values (in percents)
    unsigned int row_percent = 0;
    //all relative values
    unsigned int row_relative = 0;


    class Row
    {
    private:
        u_short height = 0;
        u_short height_offset = 0;

        //all absolute values
        unsigned int window_absolute = 0;
        //all percent values (in percents)
        unsigned int window_percent = 0;
        //all relative values
        unsigned int window_relative = 0;


        DisplayMode mode = DisplayMode::ABSOLUTE;
        void initializer(){
            switch (mode)
            {
            case DisplayMode::PERCENT:
                if(height > 100){
                    height = 100;
                }
                row_percent += height;
                break;
            case DisplayMode::RELATIVE:
                row_relative += height;
                break;
            case DisplayMode::ABSOLUTE:
                row_absolute += height;
                break;
            default:
                break;
            }
        }
        void deinitializer(){
            switch (mode) {
                case DisplayMode::PERCENT:
                    row_percent -= height;
                    break;
                case DisplayMode::RELATIVE:
                    row_relative -= height;
                    break;
                case DisplayMode::ABSOLUTE:
                    row_absolute -= height;
                    break;
                default:
                    break;
            }
        }

        unsigned int size_percent(unsigned int h = 0){
            if(h == 0){
                h = height;
            }
            return std::floor(h/static_cast<double>(100)) * HEIGHT;
        }
        unsigned int size_absolute(unsigned int h = 0){
            if(h == 0){
                h = height;
            }
            return h;
        }
        unsigned int size_relative(unsigned int h = 0){
            if(h == 0){
                h = height;
            }
            if(!row_relative){return 0;}
            return std::floor(static_cast<double>(h)/static_cast<double>(row_relative)) * HEIGHT - size_absolute(row_absolute) - size_percent(row_percent);
        }
        
        void window_initializer(u_short w, DisplayMode dm){
            switch (mode)
            {
            case DisplayMode::PERCENT:
                if(w > 100){
                    w = 100;
                }
                window_percent += w;
                break;
            case DisplayMode::RELATIVE:
                window_relative += w;
                break;
            case DisplayMode::ABSOLUTE:
                window_absolute += w;
                break;
            default:
                break;
            }
        }

        void window_deinitializer(u_short w, DisplayMode dm){
            switch (mode)
            {
            case DisplayMode::PERCENT:
                window_percent -= w;
                break;
            case DisplayMode::RELATIVE:
                window_relative -= w;
                break;
            case DisplayMode::ABSOLUTE:
                window_absolute -= w;
                break;
            default:
                break;
            }
        }
        
        std::vector<Window*> windows;
    public:
        uint size(){
            switch (mode)
            {
            case DisplayMode::PERCENT:
                return size_percent();
            case DisplayMode::RELATIVE:
                return size_relative();
            case DisplayMode::ABSOLUTE:
            default:
                return size_absolute();
            }
        }
        void resize(u_short h = 0, DisplayMode m = DisplayMode::UNDEFINED){
            if(h==0){
                h = height;
            }
            if(m == DisplayMode::UNDEFINED){
                m = mode;
            }
            deinitializer();
            height = h;
            mode = m;
            initializer();
        }
        
        Window* append(u_short w, DisplayMode dm = DisplayMode::ABSOLUTE){
            window_initializer(w, dm);
            auto ptr = new Window(0, height_offset, w, size());
            windows.push_back(ptr);
            return ptr;
        }
        Row(u_short h, DisplayMode m): height(h), mode(m) {
            initializer();
        }
        ~Row() {}
    };


    class Row2
    {
    private:
        std::vector<Window*> windows;
    public:
        u_short h = 0;
        u_short y = 0;
        Window* append(u_short w){
            auto ret = new Window(0,0, w, h);
            windows.push_back(ret);
            return ret;
        }

        void refresh(u_short _h, u_short _y){
            h = _h;
            y = _y;
            u_char x = 0;
            for (auto wind : windows) {
                wind->h = h;
                wind->y = y;
                wind->x = x;
                x += wind->w;
            }
        };

        Row2(u_short _h, u_short _y) : h(_h), y(_y) {}
        ~Row2() {}
    };

} // namespace wm



#endif // C_ROW
