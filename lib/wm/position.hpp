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

        bool operator==(const Position& p){
            return p.x == this->x && p.y == this->y;
        }

        //bool inside(wm::Position){
        //    
        //}
    };
    
} // namespace wm
