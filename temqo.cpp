#include "temqo.hpp"
#include "lib/wm/getch.hpp"
#include "lib/wm/mouse.hpp"
#include <chrono>
#include <iostream>
#include <unistd.h>





int main(int argc, char** const argv)
{
    temqo::clog << "Starting\n";
    temqo::init(argc, argv);
    temqo::clog << "Initialized\n";


    while (auto ch = wm::getch())
    {
        if(ch == 'q')
            break;
        if(is_mouse(ch) && wm::parse_mouse(ch).valid){
            auto m = wm::parse_mouse(ch);
            temqo::mpos = m.pos;
            temqo::action(m);
        }
        else if(auto k = wm::is_key(ch)){
            temqo::action(k);
        }
        else{
            temqo::action(ch);
        }
    }
    
    //temqo::deinit();
    try{
        temqo::p.save();
    }catch(...){}

    temqo::clog << "Saved, Now tryna shutdown le thread..." << std::endl;
    temqo::draw = false;
    if(temqo::draw_thread)
        temqo::draw_thread->join();
    temqo::clog << "Saved, le thread was shutdown correctly..." << std::endl;

    temqo::deinit();
    //segfaults here?
    temqo::clog << "Deinit was called and executed..." << std::endl;
    return 0;
}
