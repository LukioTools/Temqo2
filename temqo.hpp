#pragma once

#include "custom/enum.hpp"
#include "lib/wm/globals.hpp"


//all of the shit that need defining

namespace temqo
{
    namespace refresh
    {
        void elements();
        void playlist();
        void progressbar();
        void controls();
        void title();
        void coverart();

        void all(){
            
        }
    } // namespace refresh

    struct DisplayMode
    {

        static double cutoff;

        enum DisplayModeEnum : unsigned char{
            AUTO,
            VERTICAL,
            HORIZONTAL,
        } mode;
        
        bool vertical(){
            return mode == HORIZONTAL || aspect_ratio >= cutoff;
        };
        bool horizontal(){
            return mode == VERTICAL || aspect_ratio < cutoff;
        };
        #undef whratio
    
    } display_mode;


    double DisplayMode::cutoff = 2;
    


    inline void init(int argc, const char** argv){

    }

} // namespace temqo

