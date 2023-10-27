#pragma once


#include "padding.hpp"
#include "space.hpp"
namespace wm
{
    enum DisplayMode : u_char{   
        UNDEFINED,
        ABSOLUTE,
        RELATIVE,
        PERCENT,
    };

    class Element
    {
    public:
        Space* space;
        Padding pad;

        inline Space wSpace(){
            return *space+pad;
        }
        inline Space aSpace(){
            return *space;
        }
        inline Padding padding(){
            return pad;
        }
        //useless
        //inline bool inside(Position pos){
        //    return space->inside(pos);
        //}
        //inline bool ainside(Position pos){
        //    return space->inside(pos);
        //}
        //inline bool winside(Position pos){
        //    return wSpace().inside(pos);
        //}
        inline operator Space(){
            return *space;
        }

        Element(Space* ptr, Padding p = {}): space(ptr), pad(p) {}
        ~Element() {}
    };
} // namespace wm

