#pragma once
/*
#include "element.hpp"
#include <vector>
namespace wm
{
    class Row;
    class Grid;

    class RowElement : public Element
    {
    private:
    public:
        Row* parent_row;
        DisplayMode dm;

        RowElement(Element el): Element(el) {}
        ~RowElement() {}
    };

    class Row
    {
    private:
        unsigned short h;
        Grid* grid_parent;

        struct allocated_sizes
        {
            unsigned int absolute = 0;
            unsigned int percent = 0;
            unsigned int relative = 0;
        };
        
        
        
    public:
        std::vector<RowElement> row_elements;
        DisplayMode dm;

        allocated_sizes count(){
            allocated_sizes szes;
            for (auto el : row_elements)
            {
                switch (el.dm)
                {
                case DisplayMode::PERCENT:  szes.percent+=el.aSpace().width();break;
                case DisplayMode::RELATIVE: szes.relative+=el.aSpace().width();break;
                case DisplayMode::ABSOLUTE:
                case DisplayMode::UNDEFINED:
                default:
                    szes.absolute+=el.aSpace().width();break;
                }
            }
            return szes;
        }

        unsigned int height(){
            switch (dm)
            {
            case DisplayMode::ABSOLUTE: return h;
            case DisplayMode::PERCENT:  return (static_cast<double>(h)/100.); //
            case DisplayMode::RELATIVE: return (static_cast<double>(h)/1 ); //
            case DisplayMode::UNDEFINED:
            default:
                return h;
            }
        }


        Row(Grid* p): grid_parent(p) {}
        ~Row() {}
    };



} // namespace wm


*/