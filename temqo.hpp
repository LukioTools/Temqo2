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
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
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
#include "lib/wm/space.hpp"

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

    static float volume_shift = 5;
    static float volume_reset = 100;
    static double playlist_coverart_ratio = 0.5;
    wm::Position mpos;

    class Valid
    {
    public:
        virtual void refresh(){};
        virtual bool is_valid(){return true;};
    };

    namespace load
    {   
        inline void file(const std::string&);
        inline void prev();
        inline void next();
        inline void curr();
    } // namespace load

    namespace control
    {
        inline void playpause();
        inline void play();
        inline void pause();
        inline void shuffle();
        inline void sort();
    } // namespace control
    

    
    

    class DisplayMode
    {
        public:

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

    class Config : public Valid
    {
        public:
        StringLite path = "./temqo.cfg";
        bool valid = false;

        void configuration(){//do the config
            cfg::il_cfg.clear();
            cfg::ml_cfg.clear();
            //do the configs again
        }

        bool is_valid() override{
            return valid;
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

    class Playlist : public audio::Playlist, public Valid {
    public:
        size_t cursor_offset = 0;
        size_t display_offset = 0;
        bool filename_only = true;
        wm::Element element;
        wm::SPLICE_TYPE clip_type = wm::SPLICE_TYPE::BEGIN_DOTS;
        wm::PAD_TYPE pad_type = wm::PAD_TYPE::PAD_RIGHT;
        bool valid = false;

        //remember to implement the lööp

        enum LoopType : unsigned char {
            L_NONE, L_TRACK, L_PLAYLIST
        } loopType = L_PLAYLIST;

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
            //auto as = element.aSpace();
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
        bool is_valid() override{
            return valid;
        }
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
    public:
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


        StringLite char_first = "├";
        StringLite char_last = "┤";
        StringLite char_center = "─";
        StringLite char_cursor = "•";

        Color color_played;
        Color color_remaining;
        Color color_cursor;

        bool valid = false;

        wm::Element element;


        void action(wm::MOUSE_INPUT m);//do something with the input
        bool is_valid() override{
            return valid;
        }
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

    struct CoverArtData
    {
        bool img_valid = false;
        bool override = false;
        StringLite file_path;
        static StringLite placeholder_path;
        static StringLite cache_path;
    };

    inline void fetch_coverart(CoverArtData* ptr){
        ptr->img_valid = true;
        auto o = p.opt_curr();
        if(ptr->override || p.empty() || o.has_value()){
            ptr->file_path = ptr->placeholder_path.get_p();
        }
        auto res = audio::extra::extractAlbumCoverTo(*o, ptr->cache_path);
        ptr->file_path = ((res == 0) ? ptr->cache_path : ptr->placeholder_path).get_p();
    }
    

    class CoverArt: public Valid{
    public:
        wm::Element element;
        CoverArtData data;
        bool valid = false;

        inline void draw_border(){
            use_attr(attr_reset << color_color(colors.border));
            if(display_mode.horizontal())
                element.space.box("─", "─", nullptr, "│", "┬", nullptr, "┴", nullptr);
            else if(display_mode.vertical())
                element.space.box("─", "─", nullptr, nullptr, "─", "─", "─", "─");
        };


        inline void draw_image(){
            if(!std::filesystem::exists(data.file_path.get_p()))
                return;

            auto s = element.wSpace();
            ascii_img::load_image_t *cover_ansi = audio::extra::getImg(data.file_path, s.width(), s.height() + 1);
            
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
                
        };


        bool is_valid() override{
            return valid && data.img_valid;
        };

        void refresh() override{
            valid = true;
            std::thread* thr = data.img_valid ? nullptr : new std::thread(fetch_coverart, &data); 
            draw_border();
            if(thr)
                thr->join();
            draw_image();
        };

    } cover_art;






    ENUM(IDS, unsigned int,
        NO_ID,
        PREV,
        NEXT,
        SHUFFLE,
        PLAYPAUSE,
        LOOP,
        VOLUME,
        TIME
    );

    ENUM(GROUPS, short,
        UNGROUPED,
        MEDIA_CONTROLS,
        VOLUME,
        TIME
    );

    namespace ControlButtons
    {
        StringLite prev_ch = "⏮";
        ButtonArrayElement prev_bae({
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr( attr_reset );
                if(inside){
                    use_attr(color_color(colors.hover)); 
                    if(m.btn == wm::MOUSE_BTN::M_LEFT)
                        load::prev();
                }
                else
                    use_attr(color_color(colors.normal))
                
                std::cout << prev_ch << ' ' << attr_reset;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::PREV,
        });

        StringLite next_ch = "⏭";
        ButtonArrayElement next_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT)
                        load::next();
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << next_ch << ' ' << attr_reset;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::NEXT,
        };

        StringLite playing_ch = "▶";
        StringLite stopped_ch = "⏸";

        ButtonArrayElement playpause_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                auto& usech = audio::playing() ? playing_ch : stopped_ch;
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT && p.size())
                        control::playpause();
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << usech << ' ' << attr_reset;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::PLAYPAUSE,
        };

        StringLite shuffle_ch = "⤮";
        StringLite sort_ch = "⇉";

        ButtonArrayElement shuffle_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                auto& usech = p.sorted() ? sort_ch : shuffle_ch;
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT && p.size()){
                        if(p.sorted())
                            control::shuffle();
                        else
                            p.sort();
                    }
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << usech << ' ' << attr_reset;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::SHUFFLE,
        };

        StringLite loop_track_ch = "⥀";
        StringLite loop_playlist_ch = "♺";
        StringLite loop_off_ch = "🡢";

        ButtonArrayElement loop_bae = {
        [](bool inside, wm::MOUSE_INPUT m)
            {
                use_attr(attr_reset);
                StringLite &use_char = p.loopType == p.L_NONE ? loop_off_ch : p.loopType == p.L_PLAYLIST? loop_playlist_ch : loop_track_ch;
                
                if (inside)
                {
                    use_attr(color_color(colors.hover));
                    if (m.btn == wm::MOUSE_BTN::M_LEFT)
                    {
                        if(p.loopType == p.L_NONE){
                            p.loopType = p.L_PLAYLIST;
                        }else if(p.loopType == p.L_PLAYLIST){
                            p.loopType = p.L_TRACK;
                        }else if(p.loopType == p.L_TRACK){
                            p.loopType = p.L_NONE;
                        }
                    }
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << use_char << " " << attr_reset;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::LOOP
        };

        ButtonArrayElement volume_bae = {
    [](bool inside, wm::MOUSE_INPUT m)
        {
            use_attr(attr_reset);
            // functionality
            if (inside)
            {
                use_attr(color_color(colors.hover));

                if (m.btn == wm::MOUSE_BTN::M_SCRL_UP)
                {
                    audio::volume::shift(volume_shift);
                }
                else if (m.btn == wm::MOUSE_BTN::M_SCRL_DOWN)
                {
                    audio::volume::shift(-volume_shift);
                }
                else if (m.btn == wm::MOUSE_BTN::M_LEFT)
                {
                    audio::volume::set(volume_reset);
                }
            }
            else
                use_attr(color_color(colors.normal));

            // drawing
            int vol = std::abs( (int) std::round(audio::volume::get()) );
            vol = vol > 999 ? 999 : vol;
            std::cout << std::setfill('0') << std::setw(3) << vol << '%' << ' ' << attr_reset;

        },
        5,
        GROUPS::VOLUME,
        IDS::VOLUME
        };

        ButtonArrayElement time_bae = {
            [](bool inside, wm::MOUSE_INPUT m){},
            11,
            GROUPS::TIME,
            IDS::TIME,
        };




    } // namespace ControlButtons
    
    static std::vector<ButtonArrayElement> controlButtons({
        ControlButtons::prev_bae,
        ControlButtons::playpause_bae,
        ControlButtons::next_bae,
        ControlButtons::shuffle_bae,
        ControlButtons::loop_bae,
        ControlButtons::volume_bae,
    });

    class Controls :public ButtonArray, public Valid{
    public:

        void action(wm::MOUSE_INPUT m){
            draw(m);
        };
        //why? idk man...
        void invalidate_all(){
            for (auto e : *this) {
                e.valid = false;
            }
        }

        bool is_valid() override{
            for (auto e : *this) {
                if(!e.valid)
                    return false;

            }
            return true;
        }

        void refresh() override{
            this->draw({
                mpos,
                wm::MOUSE_BTN::M_NONE,
                true,
                });
        }

        Controls() = default;
        Controls(const std::vector<ButtonArrayElement>& v): ButtonArray(v) {};

        ~Controls() = default;

    } ctrl(controlButtons);

    struct Title : public Valid
    {
        wm::Element element;
        bool scr_title_filename_only;
        bool window_title_filename_only;
        bool valid = false;

        bool is_valid() override{
            return valid;
        }

        void refresh() override{
            valid = true;
            if(p.empty())
                return;
            //set window title
            //maby set metadata also but well do it later or something :3
            use_attr(set_title_stream((window_title_filename_only ? path::filename(p.current()) : p.current())));
            
            std::string c = scr_title_filename_only ? path::filename(p.current()) : p.current();
            auto s = element.aSpace();
            // current_file.space.fill("?");
            wm::clip(c, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
            wm::pad(c, s.width(), wm::PAD_TYPE::PAD_CENTER);
            mv(s.x, s.y);
            std::cout << c;
        }
    } title;
    


    
    
    

    

    namespace refresh
    {
        //the element size config

        inline void elements_horizontal(){
            p.element.space = {
                0,
                title.element.space.h, 
                static_cast<unsigned short>(wm::WIDTH*playlist_coverart_ratio),
                static_cast<unsigned short>(wm::HEIGHT-title.element.space.h - progres_bar.element.space.h)
            };
            p.element.pad = {1, 1, 0, 1};

            cover_art.element.space = {
                p.element.space.w,
                title.element.space.h, 
                static_cast<unsigned short>(wm::WIDTH-p.element.space.w+1),
                static_cast<unsigned short>(wm::HEIGHT-title.element.space.h - progres_bar.element.space.h)
            };
            cover_art.element.pad = {1, 1, 1, 0};
        }

        inline void elements_vertical(){
            auto useable_height = (wm::HEIGHT-title.element.space.h-progres_bar.element.space.h);
            p.element.space = {
                0,
                title.element.space.h, 
                static_cast<unsigned short>(wm::WIDTH),
                static_cast<unsigned short>(useable_height*playlist_coverart_ratio)
            };
            p.element.pad = {1, 1, 0, 1};

            cover_art.element.space = {
                p.element.space.w,
                title.element.space.h, 
                static_cast<unsigned short>(wm::WIDTH),
                static_cast<unsigned short>(useable_height-p.element.space.h)
            };
            cover_art.element.pad = {1, 1, 1, 0};
        }
        //every resize_event
        inline void elements(){
            title.element.space = wm::Space(0,0, wm::WIDTH, 1);

            ctrl.pos = {static_cast<unsigned short>(wm::WIDTH - ctrl.width()), static_cast<unsigned short>(wm::HEIGHT)};
            progres_bar.element.space = wm::Space(0, wm::HEIGHT, ctrl.pos.x - 1 , 1);
            
            if(display_mode.horizontal())
                elements_horizontal();
            else if(display_mode.vertical())
                elements_vertical();
        };

        //all the classes that have the refresh thing
        inline void playlist(){
            if(!p.is_valid())
                p.refresh();
        };
        inline void progressbar(){
            if(!progres_bar.is_valid())
                progres_bar.refresh();
        };
        inline void coverart(){
            if(!cover_art.is_valid())
                cover_art.refresh();
        };
        inline void config(){
            if(!cfg.is_valid())
                cfg.refresh();
        };
        //todo
        inline void controls(){
            if(!ctrl.is_valid())
                ctrl.draw({mpos, wm::MOUSE_BTN::M_NONE, true});
        };
        inline void title(){
            if(p.empty())
                return;
            
        };

        inline void all(){
            if(wm::resize_event){
                p.valid = false;
                progres_bar.valid = false;
                cover_art.valid = false;
            }
            elements();
            playlist();
            progressbar();
            controls();
            title();
            coverart();
        };

    } // namespace refresh

    namespace load
    {
        inline void file(const std::string& filepath){
            if(!filepath.size() || std::filesystem::exists(filepath))
                return;
            //invalidate shit and stuff

            audio::load(filepath);
            p.valid = false;
            cover_art.valid = false;
            cover_art.data.img_valid = false;
            ctrl.invalidateGroup(GROUPS::TIME);
            ctrl.invalidateGroup(GROUPS::MEDIA_CONTROLS);
        }

        inline void next(){
            auto o = p.opt_next();
            if(!o.has_value())
                return;
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

    namespace control
    {
        inline void playpause(){
            if(p.empty())
                return;
            
            audio::control::toggle();
            ctrl.invalidateId(IDS::PLAYPAUSE);
        }
        inline void play(){
            if(p.empty())
                return;

            audio::control::play();
            ctrl.invalidateId(IDS::PLAYPAUSE);
        }
        inline void pause(){
            if(p.empty())
                return;

            audio::control::pause();
            ctrl.invalidateId(IDS::PLAYPAUSE);
        };
        inline void shuffle(){
            if(p.empty())
                return;
            if(!p.seed)
                p.new_seed();
            p.shuffle();
            ctrl.invalidateId(IDS::SHUFFLE);
        }
        inline void sort(){
            if(p.empty())
                return;
            p.sort();
            ctrl.invalidateId(IDS::SHUFFLE);
        }

    } // namespace control
    
    
    
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

    std::mutex drawing;
    bool draw = true;

    inline void at_exit(){
        draw = false;
        audio::control::pause();
        wm::deinit();
    }
    inline void at_int(int){
        at_exit();
        exit(0);
    }


    inline void draw_thread(){
        while (draw)
        {
            std::lock_guard lock(drawing);
            refresh::all();
        }
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
                control::shuffle();
            
            load::curr();
        }

    }

    inline void deinit(){
        at_exit();
    }

} // namespace temqo

