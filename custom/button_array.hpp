#include "../lib/wm/element.hpp"
#include "../lib/wm/mouse.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

struct ButtonArrayElement
{
    void(*draw)(bool inside, wm::MOUSE_INPUT m);//the function that prints out the data //plz use the whole space as not to induse any oddities
    short alloc;
};

template<typename T = ButtonArrayElement>
class ButtonArray : public std::vector<T>
{
private:
    
public:
    wm::Position pos; // position to start printing

    void draw(wm::MOUSE_INPUT m){
        auto mpos = m.pos;
        unsigned int offset = 0;
        bool same_height = mpos.y == pos.y;

        for (size_t i = 0; i < this->size(); i++)
        {
            T e = this->at(i);
            auto poffset = pos.x+offset;


            bool inside_x_axis = mpos.x >= poffset && mpos.x < poffset+e.alloc;

            mv(poffset, pos.y);
            if(!e.draw)
                continue;
            offset+=e.alloc;
            //std::cout << i;
            e.draw(same_height && inside_x_axis, m);
        }
    }

    

    size_t width(){
        size_t out  = 0;
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


