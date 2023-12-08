#include "temqo.hpp"
#include "clog.hpp"
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
        //temqo::clog << "input_mode: " << (int) temqo::input_mode.num << std::endl;
        if(temqo::input_mode == temqo::InputMode::COMMON){
            if(ch == 'q')
                break;
            else if (ch == ':') {
                temqo::clog << "MODE IS NOW COMMAND" << std::endl;
                temqo::input_mode = temqo::InputMode::COMMAND;
                temqo::input.clear();
            }
            else if (ch == '/') {
                temqo::input_mode = temqo::InputMode::SEARCH;
                temqo::input.clear();
            }
            else if(is_mouse(ch) && wm::parse_mouse(ch).valid){
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
        else if (temqo::input_mode == temqo::InputMode::COMMAND || temqo::input_mode == temqo::InputMode::SEARCH ) {
            if(ch == 0x1b1b){
                temqo::input_mode = temqo::InputMode::COMMON;
                temqo::clog << "SET TO COMMON FROM ESC CHARACHER" << std::endl;
                temqo::input.clear();
            }
            else if(is_mouse(ch) && wm::parse_mouse(ch).valid){
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
