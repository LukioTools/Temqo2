#pragma once

#include "def.hpp"
#include "padding.hpp"
#include "position.hpp"
#include <iostream>


namespace wm
{
        struct Space{
        unsigned short x = 0;
        unsigned short y = 0;
        unsigned short w = 0;
        unsigned short h = 0;

        inline void fill(const char* ch){
            auto s = start();
            auto e = end();
            
            for (size_t y = s.y; y <= e.y; y++)
            {
                mv(s.x, y);
                for (size_t x = s.x; x <= e.x; x++)
                {
                    std::cout << ch;
                }
            }
        }
        inline Position start(){
            return {x,y};
        }
        inline Position end(){
            return {static_cast<unsigned short>(x+w), static_cast<unsigned short>(y+h)};
        }
        inline unsigned short max_x(){
            return static_cast<unsigned short>(x+w);
        }
        inline unsigned short max_y(){
            return static_cast<unsigned short>(y+h);
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
            w-=ammount;
        }
        void expand_left(int ammount){
            x-=ammount;
        }
        void expand_top(int ammount){
            y-=ammount;
        }
        void expand_bottom(int ammount){
            h-=ammount;
        }
        Space(unsigned short _x, unsigned short _y, unsigned short _w, unsigned short _h): x(_x), y(_y), w(_w) , h(_h) {}
        inline void refresh(unsigned short _x, unsigned short _y, unsigned short _w, unsigned short _h){
            x = _x;
            y = _y;
            w = _w;
            h = _h;
        }

        inline bool inside(unsigned short _x, unsigned short _y){
            return !((_x < x || _x > x+w) || (_y < y || _y > y+h));
        }

        inline bool inside(wm::Position pos){
            return !((pos.x < x || pos.x > x+w) || (pos.y < y || pos.y > y+h));
        }

#define if_not_mv_left(val) (val ? val : cursor_left(1))
        int box(const char* t = "─", const char* b = "─", const char* r = "│", const char* l = "│", const char* lt = "┌", const char* rt = "┐",const char* lb = "└", const char* rb = "┘"){
            if(!exists()){
                return -1;
            }

            auto s = start();
            auto e = end();

            if(lt){
                mv(s.x, s.y);
                std::cout << lt;
            }
            if(rt){
                mv(e.x, s.y);
                std::cout << rt;
            }
            if(t){
                for (size_t x = s.x+1; x < e.x; x++)
                {   
                    mv(x , s.y);
                    std::cout << t;
                }
            }

            if(lb){
                mv(s.x, e.y);
                std::cout << lb;
            }
            if(rb){
                mv(e.x, e.y);
                std::cout << rb;
            }
            if(b){
                for (size_t x = s.x+1; x < e.x; x++)
                {   
                    mv(x, e.y);
                    std::cout << b;
                }
            }
            
            



            for (size_t y = s.y; y <= e.y; y++)
            {
                if(y == s.y || y == e.y){
                    continue;
                }
                if(l){
                    mv(s.x, y)
                    std::cout << l;
                }
                if(r){
                    mv(e.x, y)
                    std::cout << r;
                }
            }

            return 0;
        }
        
#undef if_not_mv_left

        Space operator+(Padding pad){
            return Space(x + pad.l, y + pad.t, (w-pad.l)-pad.r, (h-pad.b)-pad.t);
        }
        friend std::ostream& operator<<(std::ostream& os, const Space& dt) {
            os <<"x:" << dt.x <<" y:" << dt.y << " w:" << dt.w << " h:" << dt.h;
            return os;
        };
        bool exists(){
            return !(w == 0 || h == 0);
        }
    };


} // namespace wm
