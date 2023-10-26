#pragma once

#include <sys/types.h>
#include "position.hpp"

#define SET_X10_MOUSE               9
#define SET_VT200_MOUSE             1000
#define SET_VT200_HIGHLIGHT_MOUSE   1001
#define SET_BTN_EVENT_MOUSE         1002
#define SET_ANY_EVENT_MOUSE         1003

#define SET_FOCUS_EVENT_MOUSE       1004

#define SET_ALTERNATE_SCROLL        1007

#define SET_EXT_MODE_MOUSE          1005
#define SET_SGR_EXT_MODE_MOUSE      1006
#define SET_URXVT_EXT_MODE_MOUSE    1015
#define SET_PIXEL_POSITION_MOUSE    1016

#define USE_MOUSE SET_ANY_EVENT_MOUSE


#define enable_mouse(type) ("\e[?"+     std::to_string(type)    +"h")
#define disable_mouse(type) ("\e[?"+    std::to_string(type)    +"l")

#define is_mouse(inp) (reinterpret_cast<char*>(&inp)[0] == '\xFF')


namespace wm 
{
    ///!!NOT GUARANTEED!!///
    enum MOUSE_BTN : unsigned char{
        M_UNDEFINED = 0,
        M_NONE = '\x42',
        M_RELEASE = '\x22',
        M_LEFT = '\x1f',
        M_MIDDLE = '\x20',
        M_RIGHT = '\x21',
        M_SCRL_UP = '\x5F',
        M_SCRL_DOWN = '\x60',
        M_LEFT_HILIGHT = '\x3f',
        M_MIDDLE_HILIGHT = '\x40',
        M_RIGHT_HILIGHT = '\x41',
    };



    struct MOUSE_INPUT {
        wm::Position pos;
        MOUSE_BTN btn = MOUSE_BTN::M_UNDEFINED;
        u_char valid = 1;
    };

    

    static MOUSE_INPUT parse_mouse(int input) noexcept{
        auto ptr = reinterpret_cast<unsigned char*>(&input);
        char checksum = static_cast<char>(ptr[0]);
        MOUSE_BTN btn = static_cast<MOUSE_BTN>(ptr[1]);
        u_char x = ptr[2];
        u_char y = ptr[3];
        return {x,y,btn,checksum == '\xFF'};
    }


} // namespace mouse
