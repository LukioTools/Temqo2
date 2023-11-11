#include "../lib/wm/element.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

struct ButtonArrayElement
{
    void(*draw)(bool inside);//the function that prints out the data //plz use the whole space as not to induse any oddities
    short alloc;
};

template<typename T = ButtonArrayElement>
class ButtonArray : public std::vector<T>
{
private:
    
public:
    wm::Position pos; // position to start printing

    void draw(wm::Position mpos = {0,0}){
        unsigned int offset = 0;
        for (size_t i = 0; i < this->size(); i++)
        {
            auto estart = pos.x+offset;
            mv(estart, pos.y);
            T e = this->at(i);
            if(!e.draw)
                continue;
            e.draw((mpos.y == pos.y) && (mpos.x >= estart && mpos.x <= estart + e.alloc));
            offset+=e.alloc;
        }
    }

    size_t width(){
        size_t out;
        for (size_t i = 0; i < this->size(); i++)
        {
            T e = this->at(i);
            out += e.alloc;
        }
        return out;
    }

    bool inside(wm::Position p){
        if(p.y != pos.y){
            return false;
        }
        if(p.x < pos.x){
            return false;
        }
        if(p.x > pos.x+width()){
            return false;
        }
        return true;
    }



    ButtonArray(){};
    ButtonArray(std::initializer_list<T> ls): std::vector<T>(ls){}
    ~ButtonArray() {}
};


