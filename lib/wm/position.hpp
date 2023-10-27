#pragma once

#include <ostream>
namespace wm
{
    struct Position{
        unsigned short x = 0;
        unsigned short y = 0;

        friend std::ostream& operator<<(std::ostream& stream, Position pos){
            stream << '(' << pos.x << ", " <<pos.y << ')';
            return stream;
        };
    };
    
} // namespace wm
