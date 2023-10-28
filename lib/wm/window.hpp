#pragma once
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sys/types.h>
#include <vector>
#include "element.hpp"

namespace wm
{
    
    
    
        //Simple position holder
    struct Window : public Element
    {


        Window(Space* ptr, Padding p = {}): Element(ptr, p) {}
        ~Window() {}
    };

} // namespace wm

 
