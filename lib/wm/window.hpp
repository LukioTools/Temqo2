#include <cmath>
#include <fstream>
#include <iterator>
#include <sys/types.h>
#include <vector>
#include "globals.hpp"
#include "position.hpp"

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
    struct Padding{
        u_char t = 1;
        u_char b = 1;
        u_char l = 2;
        u_char r = 2;

        void pad(){
            t = 1;
            b = 1;
            l = 2;
            r = 2;
        }
        void pad(u_char v){
            t = v;
            b = v;
            l = v;
            r = v;
        }
        void pad(u_char tb, u_char lr){
            t = tb;
            b = tb;
            l = lr;
            r = lr;
        }
        void pad(u_char _t, u_char _b, u_char _l,u_char _r){
            t = _t;
            b = _b;
            l = _l;
            r = _r;
        }
        friend std::ostream& operator<<(std::ostream& os, const Padding& dt) {
            os <<"t:" << (int) dt.t <<" b:" << (int) dt.b << " l:" << (int) dt.l << " r:" << (int) dt.r;
            return os;
        };
    };

    struct Space{
        u_short x = 0;
        u_short y = 0;
        u_short w = 0;
        u_short h = 0;


        Position start(){
            return {x,y};
        }
        Position end(){
            return {static_cast<unsigned short>(x+w), static_cast<unsigned short>(y+h)};
        }

        unsigned short width(){
            return w;
        }
        unsigned short height(){
            return h;
        }

        void transform_vertical(int ammount){
            y+=ammount;
            h-=ammount;
        }

        void transform_horizontal(int ammount){
            x+=ammount;
            w-=ammount;
        }
        /*negative values shrink it*/
        void expand_right(int ammount){
            w+=ammount;
        }
        void expand_left(int ammount){
            x+=ammount;
        }

        Space(u_short _x, u_short _y, u_short _w, u_short _h): x(_x), y(_y), h(_h), w(_w) {}
        void refresh(u_short _x, u_short _y, u_short _w, u_short _h){
            x = _x;
            y = _y;
            w = _w;
            h = _h;
        }

        bool inside(u_short _x, u_short _y){
            return !((_x < x || _x > x+w) || (_y < y || _y > y+h));
        }

        bool inside(wm::Position pos){
            return !((pos.x < x || pos.x > x+w) || (pos.y < y || pos.y > y+h));
        }

        

        Space operator+(Padding pad){
            return Space(x + pad.r, y + pad.t, w-pad.l-pad.r, h-pad.b-pad.t-1);
        }
        friend std::ostream& operator<<(std::ostream& os, const Space& dt) {
            os <<"x:" << dt.x <<" y:" << dt.y << " w:" << dt.w << " h:" << dt.h;
            return os;
        };
        bool exists(){
            return !(x == 0 || y == 0 || w == 0 || h == 0);
        }
    };


        //Simple position holder
    struct Window
    {
        Space* space;
        Padding padding;
        DisplayMode dm;
        //bool flag;
        //u_short _;

        Space AbsoluteSpace() noexcept{
            if(!space){
                return {0,0,0,0};
            }
            return *space;
        }

        Space WriteableSpace() noexcept{
            if(!space){
                return {0,0,0,0};
            }
            return *space+padding;
        }


        Window & operator=(const Window&) = delete;
        Window(const Window&) = delete;

        Window(DisplayMode d, Space* ptr, Padding p = {}): dm(d), space(ptr), padding(p) {}
        ~Window() {}
    };

    /**
     * @brief Holds objects that are dynamicly sized
     * @tparam T 
     */
    template<typename T>
    class DynamicHolder
    {
    public:
        std::vector<T> data;
    protected:

        uint abs = 0;
        uint rel = 0;
        uint cnt = 0;

        //object is added
        void init(DisplayMode dm, u_short h){
            switch (dm)
            {
            case DisplayMode::PERCENT:
                if(h > 100){
                    h = 100;
                }
                cnt += h;
                break;
            case DisplayMode::RELATIVE:
                rel += h;
                break;
            case DisplayMode::ABSOLUTE:
                abs += h;
                break;
            default:
                break;
            }
        }
        //object is removed
        void deinit(DisplayMode dm, u_short h){
            switch (dm)
            {
            case DisplayMode::PERCENT:
                cnt -= h;
                break;
            case DisplayMode::RELATIVE:
                rel -= h;
                break;
            case DisplayMode::ABSOLUTE:
                abs -= h;
                break;
            default:
                break;
            }
        }

        unsigned int size_percent(unsigned int h = 0){
            return std::floor(h/static_cast<double>(100)) * HEIGHT;
        }
        unsigned int size_absolute(unsigned int h = 0){
            return h;
        }
        unsigned int size_relative(unsigned int h = 0){
            if(!rel){return 0;}
            return std::floor(static_cast<double>(h)/static_cast<double>(rel)) * HEIGHT - size_absolute(abs) - size_percent(abs);
        }

    public:

        void append(T obj, DisplayMode dm, u_short h){
            this->init(dm, h);
            data.push_back(obj);
        }
        void remove(size_t index, DisplayMode dm, u_short h){
            auto it = data.begin();
            std::advance(it, index);
            data.erase(it);
            deinit(dm, h);
        }


        DynamicHolder(/* args */) {}
        ~DynamicHolder() {}
    };
} // namespace wm

 
#endif // C_WINDOWS
