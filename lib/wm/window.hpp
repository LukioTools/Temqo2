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
    
    
    
    //useless nowadays
        //Simple position holder
    struct Window : public Element
    {


        Window(Space* ptr, Padding p = {}): Element(ptr, p) {}
        ~Window() {}
    };

    /**
     * @brief Holds objects that are dynamicly sized
     * @tparam T 
     */
    template<typename T>
    class DynamicHolder
    {
    public:
        std::vector<T> data;
    protected:

        uint abs = 0;
        uint rel = 0;
        uint cnt = 0;

        //object is added
        void init(DisplayMode dm, u_short h){
            switch (dm)
            {
            case DisplayMode::PERCENT:
                if(h > 100){
                    h = 100;
                }
                cnt += h;
                break;
            case DisplayMode::RELATIVE:
                rel += h;
                break;
            case DisplayMode::ABSOLUTE:
                abs += h;
                break;
            default:
                break;
            }
        }
        //object is removed
        void deinit(DisplayMode dm, u_short h){
            switch (dm)
            {
            case DisplayMode::PERCENT:
                cnt -= h;
                break;
            case DisplayMode::RELATIVE:
                rel -= h;
                break;
            case DisplayMode::ABSOLUTE:
                abs -= h;
                break;
            default:
                break;
            }
        }

        unsigned int size_percent(unsigned int h = 0){
            return std::floor(h/static_cast<double>(100)) * HEIGHT;
        }
        unsigned int size_absolute(unsigned int h = 0){
            return h;
        }
        unsigned int size_relative(unsigned int h = 0){
            if(!rel){return 0;}
            return std::floor(static_cast<double>(h)/static_cast<double>(rel)) * HEIGHT - size_absolute(abs) - size_percent(abs);
        }

    public:

        void append(T obj, DisplayMode dm, u_short h){
            this->init(dm, h);
            data.push_back(obj);
        }
        void remove(size_t index, DisplayMode dm, u_short h){
            auto it = data.begin();
            std::advance(it, index);
            data.erase(it);
            deinit(dm, h);
        }


        DynamicHolder(/* args */) {}
        ~DynamicHolder() {}
    };
} // namespace wm

 
