#pragma once

#include <string>
namespace wm
{
    enum SPLICE_TYPE
    {
        CUSTOM,
        END_CUT,
        END_DOTS,
        BEGIN_CUT,
        BEGIN_DOTS,
    };

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
    
    inline void clip( std::string& str_ref, unsigned char width, SPLICE_TYPE st){
        if(str_ref.length() <= width){
            return;
        }
        switch (st)
        {
        case END_DOTS:
            clip_functions::end_dots(str_ref, width);
            break;
        case BEGIN_CUT:
            clip_functions::beg_cut(str_ref, width);
            break;
        case BEGIN_DOTS:
            clip_functions::beg_dots(str_ref, width);
            break;
        case END_CUT:
        default:
            clip_functions::end_cut(str_ref, width);
            break;
        }
    }

} // namespace wm
