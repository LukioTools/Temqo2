#pragma once

#include <ostream>
namespace wm
{
    struct Padding{
        unsigned char t = 0;
        unsigned char b = 0;
        unsigned char l = 0;
        unsigned char r = 0;

        Padding(){
            t = 0;
            b = 0;
            l = 0;
            r = 0;
        }
        Padding(unsigned char v){
            t = v;
            b = v;
            l = v;
            r = v;
        }
        Padding(unsigned char tb, unsigned char lr){
            t = tb;
            b = tb;
            l = lr;
            r = lr;
        }
        Padding(unsigned char _t, unsigned char _b, unsigned char _l,unsigned char _r){
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


} // namespace wm
