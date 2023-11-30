#pragma once

#include "custom/enum.hpp"
#include "lib/audio/extract_img.hpp"
#include "lib/audio/playlist.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/cfg/config.hpp"
#include "lib/cfg/parsers.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/element.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/init.hpp"
#include <bits/getopt_core.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <signal.h>
#include <vector>
#include "custom/stringlite.hpp"
#include "lib/wm/clip.hpp"
#include "custom/button_array.hpp"
#include "lib/wm/mouse.hpp"
#include "lib/wm/position.hpp"

#define default_normal_fg {255,255,255}
#define default_normal_bg {0,0,0}

#define default_hilight_fg {20,30,20}
#define default_hilight_bg {200, 200, 200}

#define default_hover_fg {222,222,222}
#define default_hover_bg {60,60,60}

#define default_warn_fg {150,60,60}
#define default_warn_bg {0,0,0}

#define default_border_fg  {193, 119, 1}
#define default_border_bg  {0,0,0}

#define clamp_1(val) (val == 0) ? 1 : val

//all of the shit that need defining

namespace temqo
{

    ENUM(InputMode, unsigned char, DEFAULT, COMMAD, SEARCH) input_mode;
    //input buffer;
    std::string input;
    std::ofstream clog("/dev/null");

    wm::Position mpos;

    struct Valid
    {
        virtual void refresh();
        bool valid = false;
    };

    namespace load
    {   
        inline void file(const std::string&);
        inline void prev();
        inline void next();
        inline void curr();
    } // namespace load

    
    

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

    struct Config : public Valid
    {
        StringLite path = "./temqo.cfg";

        void configuration(){//do the config
            cfg::il_cfg.clear();
            cfg::ml_cfg.clear();
            //do the configs again
        }

        void refresh() override{
            valid = true;
            cfg::parse(path);
        }

    } cfg;

    struct Color{
        cfg::RGB fg = default_normal_fg;
        cfg::RGB bg = default_normal_bg;
        StringLite attr = "";
    };

    struct Colors
    {
        Color normal = {
            default_normal_fg,
            default_normal_bg,
            
        };
        Color hover = {
            default_hover_fg,
            default_hover_bg,
            italic
        };
        Color warn = {
            default_warn_fg,
            default_warn_bg,
            italic
        };
        Color border = {
            default_warn_fg,
            default_border_bg,
        };
        Color hilight = {
             default_hilight_fg,
            default_hilight_bg,
            bold,
        };
    } colors;

    struct Playlist : audio::Playlist, public Valid {
        size_t cursor_offset = 0;
        size_t display_offset = 0;
        bool filename_only = true;
        wm::Element element;
        wm::SPLICE_TYPE clip_type = wm::SPLICE_TYPE::BEGIN_DOTS;
        wm::PAD_TYPE pad_type = wm::PAD_TYPE::PAD_RIGHT;

        size_t clamp(long i){
            if (empty())
                return 0;
            while (i < 0)
                i += size();

            size_t e = static_cast<size_t>(i);
            if (e >= size())
                e = e % size();
            return e;
        }

        void draw_box(){
            auto s = element.space;
            if(display_mode.horizontal()){
                std::string b;
                for (size_t i = 0; i < s.width(); i++)
                {
                    b += "─";
                }
                use_attr(color_color(colors.border));
                mv(s.x, s.y);
                std::cout << b;
                mv(s.x, s.y + s.h);
                std::cout << b << attr_reset;
            }
            else if(display_mode.vertical()){
                auto e = s.end();
                std::string b;
                for (size_t i = 0; i < s.width(); i++)
                {
                    b += "─";
                }
                use_attr(color_color(colors.border));
                mv(s.x, s.y);
                std::cout << b;
                mv(s.x, e.y);
                std::cout << b << attr_reset;
            }
            
        }

        void draw_content(){
            auto as = element.aSpace();
            auto ws = element.wSpace();
            //draw playlist_content
            cursor_offset = clamp(cursor_offset);
            for (size_t index = 0; index <= ws.h; index++)
            {
                auto i = clamp(display_offset + index);
                auto o = opt_get(i);

                if(!o.has_value())
                    continue;
                    
                std::string str = *o;
                if (filename_only)
                    str = path::filename(str);

                auto i_str = std::to_string(i) + ' ';
                wm::clip(str, ws.width() - i_str.length(), clip_type);
                wm::pad(str, ws.width() - i_str.length(), pad_type);
                str = i_str + str;

                if (i == static_cast<unsigned int>(cursor_offset))
                    use_attr(color_color(colors.hover));
                if (i == current_index)
                    use_attr(color_color(colors.hilight));

                mv(ws.x, ws.y + index);
                std::cout << str << attr_reset;
            }
        }


        void action(wm::MOUSE_INPUT m);//do something with the input

        void refresh() override{
            valid = true;
            auto as = element.aSpace();
            draw_box();
            if (!size() || as.width() < 5 || as.height() < 5) return;
            draw_content();
        }

    } p;

    struct ProgressBar : public Valid
    {
    private:
        void refresh_mode_default(){
            auto d = audio::position::get_d();
            auto s = element.space;
            auto idx = static_cast<unsigned int>(s.w * d);
            mv(s.x, s.y);
            use_attr(color_color(color_played));
            for (size_t i = 0; i < s.width(); i++)
            {
                if (i == idx)
                    std::cout << color_color(color_cursor) << char_first << color_color(color_remaining);
                else if (i == 0)
                    std::cout << char_first;
                else if (i == static_cast<unsigned long>(s.width() - 1))
                    std::cout << char_last;
                else
                    std::cout << char_center;
            }
        }

        void refresh_mode_command(){
            auto s = element.space;
            std::string str = ':' + input;
            wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
            wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
            mv(s.x, s.y);
            std::cout << str;
        }

        void refresh_mode_search(){
            auto s = element.space;
            std::string str = '/' + input;
            wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
            wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
            mv(s.x, s.y);
            std::cout << str;
        }

        void refresh_mode_unknown(){
            auto s = element.space;
            std::string str = input;
            wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
            wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
            mv(s.x, s.y);
            std::cout << str;
        }

    public:

        StringLite char_first = "├";
        StringLite char_last = "┤";
        StringLite char_center = "─";
        StringLite char_cursor = "•";

        Color color_played;
        Color color_remaining;
        Color color_cursor;

        wm::Element element;

        void action(wm::MOUSE_INPUT m);//do something with the input

        void refresh() override{
            valid = true;
            switch (input_mode.num) {
                case InputMode::DEFAULT:
                    refresh_mode_command();
                    break;
                case InputMode::COMMAD:
                    refresh_mode_command();
                    break;
                case InputMode::SEARCH:
                    refresh_mode_search();
                    break;
                default:
                    refresh_mode_command();
                    //unknown mode
                    break;
            }
        }


    } progres_bar;

    struct CoverArt: public Valid{
        static StringLite cache_path;
        static StringLite placeholder_path;
        StringLite file_path;
        bool override = false;
        bool img_valid = false;
        wm::Element element;

        inline void draw_border();
        inline void draw_image();
        inline virtual void refresh();
    } cover_art;

    inline void fetch_coverart(CoverArt* ptr){
        ptr->img_valid = true;
        auto o = p.opt_curr();
        if(ptr->override || p.empty() || o.has_value()){
            ptr->file_path = ptr->placeholder_path.get_p();
        }
        auto res = audio::extra::extractAlbumCoverTo(*o, ptr->cache_path);
        ptr->file_path = ((res == 0) ? ptr->cache_path : ptr->placeholder_path).get_p();
    }

    inline void CoverArt::draw_border(){
        use_attr(attr_reset << color_color(colors.border));
        if(display_mode.horizontal())
            element.space.box("─", "─", nullptr, "│", "┬", nullptr, "┴", nullptr);
        else if(display_mode.vertical())
            element.space.box("─", "─", nullptr, nullptr, "─", "─", "─", "─");
    }

    inline void CoverArt:: draw_image(){
        if(!std::filesystem::exists(file_path.get_p()))
            return;

        auto s = element.wSpace();
        ascii_img::load_image_t *cover_ansi = audio::extra::getImg(file_path, s.width(), s.height() + 1);
        
        std::ostringstream out;
        for (size_t iy = 0; iy <= s.height(); iy++)
        {
            out << mv_str(s.x, s.y + iy);
            for (size_t ix = 0; ix < s.width(); ix++)
            {
                auto rgb = cover_ansi->get(ix + s.width() * iy);

                out << color_bg_str(clamp_1(rgb.r), clamp_1(rgb.g), clamp_1(rgb.b)) << ' ' << attr_reset;
            }
        }
        if (cover_ansi)
            delete cover_ansi;
        std::cout << out.str();
            
    }

    inline void CoverArt::refresh(){
        valid = true;
        std::thread* thr = img_valid ? nullptr : new std::thread(fetch_coverart, this); 
        draw_border();
        if(thr)
            thr->join();
        draw_image();
    }


    ENUM(IDS, unsigned int,
        NO_ID,
        PREV,
        NEXT,
        SHUFFLE,
        TOGGLE,
        LOOP,
        VOLUME
    );

    ENUM(GROUPS, short,
        UNGROUPED,
        MEDIA_CONTROLS,
        VOLUME
    );

    StringLite prev_ch = "⏮";
    ButtonArrayElement prev_bae({
        [](bool inside, wm::MOUSE_INPUT m){
            use_attr( attr_reset );
            if(inside){
                use_attr(color_color(colors.hover)); 
                if(m.btn == wm::MOUSE_BTN::M_LEFT){
                    load::prev();
                }
            }
            else
                use_attr(color_color(colors.normal))
            
            std::cout << attr_reset << prev_ch << ' ' << attr_reset;
        },
        2,
        GROUPS::MEDIA_CONTROLS,
        IDS::PREV,
    });
    
    

    static std::vector<ButtonArrayElement> controlButtons({

    });

    class Controls :public ButtonArray, public Valid{
    public:
        void action(wm::MOUSE_INPUT m){
            valid = true; // idk why but lets just do it
            draw(m);
        };

        void refresh() override{
            valid = true;
            this->draw({
                mpos,
                wm::MOUSE_BTN::M_NONE,
                true,
                });
        }

        Controls() = default;
        Controls(const std::vector<ButtonArrayElement>& v): ButtonArray(v) {};

        ~Controls() = default;

    } controls(controlButtons);


    
    
    

    

    namespace refresh
    {
        //the element size config
        inline void elements();

        //all the classes that have the refresh thing
        inline void playlist(){
            if(!p.valid)
                p.refresh();
        };
        inline void progressbar(){
            if(!progres_bar.valid)
                progres_bar.refresh();
        };
        inline void coverart(){
            if(!cover_art.valid)
                cover_art.refresh();
        };
        inline void config(){
            if(!cfg.valid)
                cfg.refresh();
        };
        //todo
        inline void controls();
        inline void title();

        inline void all();
        inline void invalidate(Valid& v){
            v.valid = false;
        }
        inline void invalidate_drawable(){
            invalidate(p);
            invalidate(progres_bar);
            invalidate(cover_art);
        }
        inline void invalidate_elements();
        inline void invalidate_configs(){
            cfg.valid = false;
        }
    } // namespace refresh

    namespace load
    {
        inline void file(const std::string& filepath){
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
        audio::control::pause();
        wm::deinit();
    }
    inline void at_int(int){
        at_exit();
        exit(0);
    }

    inline void init(int argc,  char** const argv){

        std::cerr.rdbuf(std::ofstream("/dev/null").rdbuf());

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

