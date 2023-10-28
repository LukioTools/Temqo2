#pragma once
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sys/types.h>
#include <vector>
#include "globals.hpp"
#include "position.hpp"
#include "padding.hpp"
#include "space.hpp"
#include "element.hpp"
#include "def.hpp"

namespace wm
{
    
    
    
        //Simple position holder
    struct Window : public Element
    {


        Window(Space* ptr, Padding p = {}): Element(ptr, p) {}
        ~Window() {}
    };

} // namespace wm

 
