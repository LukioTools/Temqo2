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
        DisplayMode dm;

        inline Space wSpace(){
            return *space+pad;
        }
        inline Space aSpace(){
            return *space;
        }
        inline operator Space(){
            return *space;
        }

        Element(DisplayMode d, Space* ptr, Padding p = {}): dm(d), space(ptr), pad(p) {}
        ~Element() {}
    };
} // namespace wm

