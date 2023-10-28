#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <mutex>
#include <condition_variable> // std::condition_variable
#include <thread>
#include <sys/ioctl.h>
#include <sstream>

#include "lib/wm/core.hpp"




typedef int event; 
inline std::string to_str_event(event inp){
    switch (inp) {
        case RESIZE_EVENT:
            return "RESIZE";
        default:
            return "UNKNOWN_EVENT";
    }
}






//you can put t, b, l, r to nullptr so that the sides will not be printe


#define ifnsp(var) var? var:" "


#undef ifnsp










std::string tst_str("");


const char* cursor = "^";

wm::Position mpos = {0,0};


bool enter = false;
int main(int argc, char const *argv[])
{
    wm::init();
    //display();
    auto space = new wm::Space(0,1,wm::WIDTH,wm::HEIGHT-1);
    auto right_click_element_space = new wm::Space(0,0, 5, 7);

    auto rc_w = new wm::Element(right_click_element_space, {1,1,1,1});
    auto w= new wm::Element(space, {1,1,2,2});
    //use_attr(cursor_invisible);
    bool show_r_window = false;
    std::string inp;
    while (true) {
        int ch = wm::getch();
        clear_scr();
        if(ch == 6939){
            break;
        }
        if(ch == RESIZE_EVENT){
            space->refresh(0,1,wm::WIDTH,wm::HEIGHT-2);
        }
        wm::MOUSE_BTN btn = wm::MOUSE_BTN::M_UNDEFINED;
        if(is_mouse(ch)){
            auto m = wm::parse_mouse(ch);
            mpos = m.pos;
            btn = m.btn;

            if(m.btn == wm::M_RIGHT){
                right_click_element_space->refresh(mpos.x, mpos.y, 15, 7);
                show_r_window = true;
            }
            if(!right_click_element_space->inside(mpos)){
                show_r_window = false;
            }


        }
        

        auto ws = w->wSpace();
        mv(0,0)
        printf("(Width: %i, Height: %i)", wm::WIDTH, wm::HEIGHT);
        std::cout << space->start() << space->end() << *space;
        mv(wm::WIDTH,wm::HEIGHT)

        //if(space->inside(mpos)){
        //    use_attr(color_bg(200,200,200) << color_fg(50,50,50))
        //    space->fill("0");
        //    use_attr(attr_reset);
        //}
        //else{
        //    space->fill("0");
        //}

        if(space->box() == -1){
            std::cout << "box error";
        };
        
        if(ws.inside(mpos)){
            use_attr(color_bg(200,200,200) << color_fg(50,50,50))
            ws.fill("#");
            use_attr(attr_reset);
        }
        else{
            ws.fill("#");
        }

        {
            std::string str = "mouse: (x: " + std::to_string(mpos.x) + ", y: " + std::to_string(mpos.y) + ")";
            mv(wm::WIDTH- str.length(), 0 );
            std::cout << str;
        }
        
        
        if(show_r_window == true){
            rc_w->aSpace().box();
            rc_w->wSpace().fill("#");
            auto ws = rc_w->wSpace();
        }
        
        
        use_attr(cursor_invisible);
        std::cout.flush();
        
        
    }
    delete w;
    delete space;
    wm::deinit();
    return 0;
}
