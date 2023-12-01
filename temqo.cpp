#include "temqo.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/wm/getch.hpp"
#include "lib/wm/mouse.hpp"
#include <chrono>
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
        if(is_mouse(ch)){
            auto m = wm::parse_mouse(ch);
            if(m.valid){
                temqo::mpos = m.pos;
            }
        }
        if(auto k = wm::is_key(ch)){
            switch (k)
            {
            case wm::KEY::K_UP:
                temqo::p.curs_down();
                break;
            case wm::KEY::K_DOWN:
                temqo::p.curs_up();
                break;
            case wm::KEY::K_LEFT:
                audio::seek::rel(std::chrono::seconds(-5));
                temqo::ctrl.invalidateGroup(temqo::GROUPS::TIME);
                break;
            case wm::KEY::K_RIGHT:
                audio::seek::rel(std::chrono::seconds(5));
                temqo::ctrl.invalidateGroup(temqo::GROUPS::TIME);
                break;
            
            default:
                break;
            }
        }
    }
    
    //temqo::deinit();
    try{
        temqo::p.save();
    }catch(...){}

    return 0;
}
