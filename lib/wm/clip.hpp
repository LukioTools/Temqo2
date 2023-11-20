#pragma once
#define _XOPEN_SOURCE
#include "../../custom/enum.hpp"


#include <codecvt>
#include <fstream>
#include <locale>
#include <string>
#include <wchar.h>

namespace wm
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    inline int correct_wcwidth(wchar_t wch){
        auto l = wcwidth(wch);
        return l == -1? 2 : l;
    }

    inline size_t correct_wswidth(const std::wstring& wstr){
        size_t out = 0;
        for (auto wch : wstr)
        {
            out += correct_wcwidth(wch);
        }
        return out;
    }
    //slow af
    inline void correct_wsubstr(std::wstring& wstr, size_t pos, size_t n = std::wstring::npos){
        std::wstring out = L"";
        size_t idk_man = 0;
        for (size_t i = 0; i < n; i++)
        {
            auto idx = pos+i;
            if(wstr.length() <= idx ){
                break;
            };
            auto wch = wstr[idx];
            idk_man += correct_wcwidth(wch);
            if(idk_man >= n){ //breeaak
                break;
            }
            out+=wch;
        }
        wstr = out;
    }

    
    ENUM(SPLICE_TYPE, unsigned char, CUSTOM, END_CUT, END_DOTS, BEGIN_CUT, BEGIN_DOTS);

    namespace clip_functions
    {
        inline void end_cut(std::wstring& str, unsigned char width){
            if(correct_wswidth(str) > width){
                correct_wsubstr(str, 0, width);
            }
        }
        inline void end_dots(std::wstring& str, unsigned char width){
            if(correct_wswidth(str) > width){
                correct_wsubstr(str, 0, width-3);
                str.append(L"...");
            }
        }

        inline void beg_cut(std::wstring& str, unsigned char width){
            auto w = correct_wswidth(str);
            if(w > width){
                correct_wsubstr(str, w-width, w);
            }
        }
        inline void beg_dots(std::wstring& str, unsigned char width){
            auto w = correct_wswidth(str);
            beg_cut(str, width);
            str.replace(0, 3, L"...");
        }
    } // namespace clip_functions

    inline void clip(std::wstring& wstr, unsigned int width, SPLICE_TYPE st){
        if(correct_wswidth(wstr) <= width){
            return;
        }
        switch (st.get())
        {
        case SPLICE_TYPE::END_DOTS:
            clip_functions::end_dots(wstr, width);
            break;
        case SPLICE_TYPE::BEGIN_CUT:
            clip_functions::beg_cut(wstr, width);
            break;
        case SPLICE_TYPE::BEGIN_DOTS:
            clip_functions::beg_dots(wstr, width);
            break;
        case SPLICE_TYPE::END_CUT:
        default:
            clip_functions::end_cut(wstr, width);
            break;
        }
    }
    
    inline void clip( std::string& str, unsigned int width, SPLICE_TYPE st){
        std::wstring wstr =   converter.from_bytes(str);
        clip(wstr, width, st);
        str = converter.to_bytes(wstr);
    }

    ENUM(PAD_TYPE,  unsigned char, PAD_CUSTOM, PAD_RIGHT, PAD_LEFT, PAD_CENTER);

    namespace pad_functions
    {
        inline void right(std::wstring& str, unsigned int width){
            str.append(width-correct_wswidth(str), ' ');
        }
        inline void left(std::wstring& str, unsigned int width){
            str.insert(str.begin(),width-correct_wswidth(str), ' ');
        }
        inline void center(std::wstring& str, unsigned int width){
            auto to_pad = width-correct_wswidth(str);
            auto pleft = to_pad/2;
            auto pright = to_pad-pleft;
            str.insert(str.begin(),pleft, ' ');
            str.append(pright, ' ');
        }


    } // namespace pad_functions
    
    inline void pad(std::wstring& wstr, unsigned int width, PAD_TYPE pt){
        if(correct_wswidth(wstr) >= width){
            return;
        }
        switch (pt.get()) {
            case PAD_TYPE::PAD_RIGHT: pad_functions::right(wstr, width); break;
            case PAD_TYPE::PAD_LEFT: pad_functions::left(wstr, width); break;
            case PAD_TYPE::PAD_CENTER: pad_functions::center(wstr, width); break;
            default:
                break;;
        }
    }
    
    inline void pad(std::string& str, unsigned int width, PAD_TYPE pt){
        std::wstring wstr = converter.from_bytes(str);
        pad(wstr, width, pt);
        str = converter.to_bytes(wstr);
    }

} // namespace wm
