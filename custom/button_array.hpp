#include "../lib/wm/element.hpp"
#include "../lib/wm/mouse.hpp"
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>
#include "../clog.hpp"

struct ButtonArrayElement
{
    void(*draw)(bool inside, wm::MOUSE_INPUT m);//the function that prints out the data //plz use the whole space as not to induse any oddities
    unsigned short alloc;
    unsigned short group = 0;
    unsigned short id = 0;
    bool(*is_valid)() = nullptr;
    void (*invalidate)() = nullptr;
};

class ButtonArray : public std::vector<ButtonArrayElement>
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
            auto& e = this->at(i);
            if(!e.draw || !e.invalidate || !e.is_valid)
                continue;

            
            auto poffset = pos.x+offset;
            offset+=e.alloc;
            //std::cout << i;
            if(!e.is_valid()){ //only draw and calculate if not valid
                bool inside_x_axis = mpos.x >= poffset && mpos.x < poffset+e.alloc;

                mv(poffset, pos.y);
                e.draw(same_height && inside_x_axis, m);
            }
            //e.valid = true;
        }
    }

    void drawByGroup(short group, wm::MOUSE_INPUT m){
        auto mpos = m.pos;
        unsigned int offset = 0;
        bool same_height = mpos.y == pos.y;

        for (size_t i = 0; i < this->size(); i++)
        {
            auto& e = this->at(i);
            if(!e.draw)
                continue;

            auto poffset = pos.x+offset;
            bool inside_x_axis = mpos.x >= poffset && mpos.x < poffset+e.alloc;

            offset+=e.alloc;

            //std::cout << i;
            if(e.group == group){

                mv(poffset, pos.y);
                e.draw(same_height && inside_x_axis, m);
            }
        }
    }

    void drawById(unsigned short id, wm::MOUSE_INPUT m){
        auto mpos = m.pos;
        unsigned int offset = 0;
        bool same_height = mpos.y == pos.y;

        for (size_t i = 0; i < this->size(); i++)
        {
            auto& e = this->at(i);
            if(!e.draw)
                continue;

            auto poffset = pos.x+offset;
            bool inside_x_axis = mpos.x >= poffset && mpos.x < poffset+e.alloc;

            offset+=e.alloc;
            //std::cout << i;
            if(e.id == id){
                
                mv(poffset, pos.y);
                e.draw(same_height && inside_x_axis, m);
            }
        }
    }

    void getInGroup(unsigned short group, void(fn)(ButtonArrayElement& bae)){
        if(!fn){
            return;
        }
        for (size_t i = 0; i < this->size(); i++)
        {
            auto& e = this->at(i);
            if(e.group == group)
                fn(e);
        }
    }

    void getInId(unsigned short id, void(fn)(ButtonArrayElement& bae)){
        if(!fn){
            return;
        }
        for (size_t i = 0; i < this->size(); i++)
        {
            auto& e = this->at(i);
            if(e.id == id)
                fn(e);
        }
    }
        //returns from the first instance
    wm::Position getPosId(unsigned short id){
        unsigned int offset = 0;
        bool found = false;
        for (size_t i = 0; i < this->size(); i++)
        {
            auto& e = this->at(i);
            if(!e.draw)
                continue;

            if(e.id == id){
                found = true;
                break;
            }

            offset+=e.alloc; //increment
        }
        if(!found){
            return {0,0};
        }

        return {static_cast<unsigned short>(pos.x+offset), pos.y};
    }
    
    void invalidateGroup(unsigned short group){
        getInGroup(group, [](ButtonArrayElement & bae){
            if(bae.invalidate)
                bae.invalidate();
        });
    }

    void invalidateId(unsigned short id){
        getInId(id, [](ButtonArrayElement & bae){
            if(bae.invalidate)
                bae.invalidate();
        });
    }

    size_t width(){
        size_t out  = 0;
        for (size_t i = 0; i < this->size(); i++)
        {
            ButtonArrayElement e = this->at(i);
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
    ButtonArray(std::initializer_list<ButtonArrayElement> ls): std::vector<ButtonArrayElement>(ls){}
    ButtonArray(std::vector<ButtonArrayElement> v): std::vector<ButtonArrayElement>(v){}
    ~ButtonArray() {}
};


