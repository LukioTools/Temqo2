#pragma once
#include "../../custom/enum.hpp"


#include <string>
namespace wm
{
    ENUM(SPLICE_TYPE, unsigned char, CUSTOM, END_CUT, END_DOTS, BEGIN_CUT, BEGIN_DOTS);

    namespace clip_functions
    {
        inline void end_cut(std::string& str, unsigned char width){
            if(str.length() > width){
                str= str.substr(0, width);
            }
        }
        inline void end_dots(std::string& str, unsigned char width){
            if(str.length() > width){
                str = str.substr(0, width - 3); //(-3 to make space for dots)
                str.append("...");
                str = str.substr(0, width);
            }
        }


        inline void beg_cut(std::string& str, unsigned char width){
            if(str.length() > width){
                str = str.substr(str.length() - width, str.length());
            }
        }
        inline void beg_dots(std::string& str, unsigned char width){
            str = str.substr(str.length() - width, str.length());
            str.replace(0, 3, "...");
            str = str.substr(0, width);
        }
    } // namespace clip_functions
    
    inline void clip( std::string& str_ref, unsigned int width, SPLICE_TYPE st){
        if(str_ref.length() <= width){
            return;
        }
        switch (st.get())
        {
        case SPLICE_TYPE::END_DOTS:
            clip_functions::end_dots(str_ref, width);
            break;
        case SPLICE_TYPE::BEGIN_CUT:
            clip_functions::beg_cut(str_ref, width);
            break;
        case SPLICE_TYPE::BEGIN_DOTS:
            clip_functions::beg_dots(str_ref, width);
            break;
        case SPLICE_TYPE::END_CUT:
        default:
            clip_functions::end_cut(str_ref, width);
            break;
        }
    }

    ENUM(PAD_TYPE,  unsigned char, PAD_CUSTOM, PAD_RIGHT, PAD_LEFT, PAD_CENTER);

    namespace pad_functions
    {
        inline void right(std::string& str, unsigned int width){
            str.append(width-str.length(), ' ');
        }
        inline void left(std::string& str, unsigned int width){
            str.insert(str.begin(),width-str.length(), ' ');
        }
        inline void center(std::string& str, unsigned int width){
            auto to_pad = width-str.length();
            auto pleft = to_pad/2;
            auto pright = to_pad-pleft;
            str.insert(str.begin(),pleft, ' ');
            str.append(pright, ' ');
        }


    } // namespace pad_functions
    

    
    inline void pad(std::string& str, unsigned int width, PAD_TYPE pt){
        if(str.length() >= width){
        }
        switch (pt.get()) {
            case PAD_TYPE::PAD_RIGHT: return pad_functions::right(str, width);
            case PAD_TYPE::PAD_LEFT: return pad_functions::left(str, width);
            case PAD_TYPE::PAD_CENTER: return pad_functions::center(str, width);
            default:
                return;
        }
    }

} // namespace wm
