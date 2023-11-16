#pragma once

#include <sys/types.h>
#include "position.hpp"
#include "def.hpp"
#include "../../custom/enum.hpp"



namespace wm 
{
    ///!!NOT GUARANTEED!!///
    ENUM(MOUSE_BTN, unsigned char, M_UNDEFINED = 0, M_NONE = '\x42',
        M_RELEASE = '\x22',
        M_LEFT = '\x1f',
        M_MIDDLE = '\x20',
        M_RIGHT = '\x21',
        M_SCRL_UP = '\x5F',
        M_SCRL_DOWN = '\x60',
        M_LEFT_HILIGHT = '\x3f',
        M_MIDDLE_HILIGHT = '\x40',
        M_RIGHT_HILIGHT = '\x41');

    //enum MOUSE_BTN : unsigned char{
    //    M_UNDEFINED = 0,
    //    M_NONE = '\x42',
    //    M_RELEASE = '\x22',
    //    M_LEFT = '\x1f',
    //    M_MIDDLE = '\x20',
    //    M_RIGHT = '\x21',
    //    M_SCRL_UP = '\x5F',
    //    M_SCRL_DOWN = '\x60',
    //    M_LEFT_HILIGHT = '\x3f',
    //    M_MIDDLE_HILIGHT = '\x40',
    //    M_RIGHT_HILIGHT = '\x41',
    //};

    inline std::string to_str(wm::MOUSE_BTN btn) noexcept{
        switch (btn.get()) {
            CASE_STR(MOUSE_BTN::M_UNDEFINED);
            CASE_STR(MOUSE_BTN::M_NONE);
            CASE_STR(MOUSE_BTN::M_RELEASE);
            CASE_STR(MOUSE_BTN::M_LEFT);
            CASE_STR(MOUSE_BTN::M_MIDDLE);
            CASE_STR(MOUSE_BTN::M_RIGHT);
            CASE_STR(MOUSE_BTN::M_SCRL_UP);
            CASE_STR(MOUSE_BTN::M_SCRL_DOWN);
            CASE_STR(MOUSE_BTN::M_LEFT_HILIGHT);
            CASE_STR(MOUSE_BTN::M_MIDDLE_HILIGHT);
            CASE_STR(MOUSE_BTN::M_RIGHT_HILIGHT);
            default:
                return "UNKNOWN";
        }
    }



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
