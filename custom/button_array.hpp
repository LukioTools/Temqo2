#include "../lib/wm/element.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

struct ButtonArrayElement
{
    void(*draw)(void);//the function that prints out the data //plz use the whole space as not to induse any oddities
    short alloc;
};

template<typename T = ButtonArrayElement>
class ButtonArray : public std::vector<T>
{
private:
    
public:
    wm::Position pos; // position to start printing

    void draw(){
        unsigned int offset = 0;
        for (size_t i = 0; i < this->size(); i++)
        {
            mv(pos.x+offset, pos.y);
            T e = this->at(i);
            if(!e.draw)
                continue;
            offset+=e.alloc;
            e.draw();
        }
    }



    ButtonArray(){};
    ButtonArray(std::initializer_list<T> ls): std::vector<T>(ls){}
    ~ButtonArray() {}
};


