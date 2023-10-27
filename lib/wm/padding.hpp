#pragma once

#include <ostream>
namespace wm
{
    struct Padding{
        unsigned char t = 1;
        unsigned char b = 1;
        unsigned char l = 2;
        unsigned char r = 2;

        void pad(){
            t = 1;
            b = 1;
            l = 2;
            r = 2;
        }
        void pad(unsigned char v){
            t = v;
            b = v;
            l = v;
            r = v;
        }
        void pad(unsigned char tb, unsigned char lr){
            t = tb;
            b = tb;
            l = lr;
            r = lr;
        }
        void pad(unsigned char _t, unsigned char _b, unsigned char _l,unsigned char _r){
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
