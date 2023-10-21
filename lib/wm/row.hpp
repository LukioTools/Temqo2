#include <cmath>
#include <sys/types.h>
#include <vector>
#if !defined(C_ROW)
#define C_ROW
#include "window.hpp"

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
    public:
        std::vector<Window*> windows;
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
        Row(u_short h, DisplayMode m): height(h), mode(m) {
            initializer();
        }
        ~Row() {}
    };
} // namespace wm



#endif // C_ROW
