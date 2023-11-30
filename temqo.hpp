#pragma once

#include "custom/enum.hpp"
#include "lib/audio/playlist.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/cfg/config.hpp"
#include "lib/cfg/parsers.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/init.hpp"
#include <bits/getopt_core.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <signal.h>

#define default_normal_fg {255,255,255}
#define default_normal_bg {0,0,0}

#define default_hover_fg {222,222,222}
#define default_hover_bg {60,60,60}

#define default_warn_fg {150,60,60}
#define default_warn_bg {0,0,0}

#define default_border_fg  {193, 119, 1}
#define default_border_bg  {0,0,0}


//all of the shit that need defining

namespace temqo
{

    std::ofstream clog("/dev/null");

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

    struct Config
    {
        std::string path = "./temqo.cfg";

        void configuration(){//do the config
            cfg::il_cfg.clear();
            cfg::ml_cfg.clear();
            //do the configs again
        }

        void refresh(){
            cfg::parse(path);
        }
    } cfg;

    


    struct color{
        cfg::RGB fg = default_normal_fg;
        cfg::RGB bg = default_normal_bg;
        //std::string attr = "";
    };

    struct colors
    {
        color normal = {
            default_normal_fg,
            default_normal_bg,
        };

        color hover = {
            default_hover_fg,
            default_hover_bg,
        };
        color warn = {
            default_warn_fg,
            default_warn_bg,
        };
    };
    


    struct Playlist : audio::Playlist {
        size_t cursor_offset = 0;
        size_t display_offset = 0;

        colors c;
        

    } p;
    

    

    namespace refresh
    {
        void elements();
        void playlist();
        void progressbar();
        void controls();
        void title();
        void coverart();

        void config();

        void all();
    } // namespace refresh

    namespace load
    {
        inline void file(std::string filepath){
            if(!filepath.size() || std::filesystem::exists(filepath))
                return;
            //invalidate shit and stuff

            audio::load(filepath);
        }

        inline void next(){
            auto o = p.opt_next();
            if(o.has_value())
                file(*o);
        }
        inline void prev(){
            auto o = p.opt_prev();
            if(o.has_value())
                file(*o);
        }
        inline void curr(){
            auto o = p.opt_curr();
            if(o.has_value())
                file(*o);
        }
    } // namespace load
    
    
    
    inline void parse_arguments(int argc, char** const argv){
        int option;
        // Process command-line options using getopt
        while ((option = getopt(argc, argv, "p:c:d:")) != -1)
        {
            switch (option)
            {
            case 'p':
                if (std::filesystem::exists(optarg))
                    p.use_file = optarg;
                break;
            case 'c':
                if (std::filesystem::exists(optarg))
                    cfg.path = optarg;
                break;
            case 'd':
                if (std::filesystem::exists(optarg))
                    clog = std::ofstream(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-c config_file -p playlist_file -d debug_file/stream]" << std::endl;
                exit(1);
            }
        }
    }

    inline void at_exit(){

    }
    inline void at_int(int){
        at_exit();
    }

    inline void init(int argc,  char** const argv){
        parse_arguments(argc, argv);
        atexit(at_exit);
        signal(SIGINT, at_int);

        wm::init();
        use_attr(cursor_invisible);

        cfg.configuration();
        cfg.refresh();

        p.use();
        if(p.size()){
            if(p.seed)
                p.shuffle();
            
            load::curr();
        }





    }

} // namespace temqo

