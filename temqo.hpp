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
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <ratio>
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
#include "clog.hpp"

#define MPRIS 1
#if defined(MPRIS)
#include "dbus/mpris_server.hpp"
#endif // MPRIS



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

    ENUM(InputMode, unsigned char, PROGRESS, COMMAD, SEARCH) input_mode = InputMode::PROGRESS;
    //input buffer;
    std::string input;

    #if defined(MPRIS)
    mpris::Server s("Temqo");
    #endif // MPRIS

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
        bool loading = false;
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
        inline void volume_set(double);
        inline void volume_rel(double);
        inline void volume_set_invalidate(double);
        inline void volume_rel_invalidate(double);
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
            return mode == VERTICAL || aspect_ratio < cutoff;
        };
        bool horizontal(){
            return mode == HORIZONTAL || aspect_ratio >= cutoff;
        };
    
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
            default_border_fg,
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
        size_t st_index()
        {
            return clamp(display_offset);
        }
        size_t last_index()
        {
            return clamp(display_offset + element.wSpace().h);
        }

        bool ref_disp_offset(){ //returns if need to redraw
            bool ret = false;
            auto idx = clamp(cursor_offset);
            auto fst = st_index();
            auto lst = last_index();
            if (idx < fst)
            {
                display_offset -= fst - idx;
                ret = true;
            }
            if (idx > lst)
            {
                display_offset -= lst - idx;
                ret = true;
            }
            return ret;
        }
        void curs_up(size_t n = 1){
            cursor_offset+=n;
            ref_disp_offset();
            valid = false;
        }
        void curs_down(size_t n = 1){
            cursor_offset-=n;
            ref_disp_offset();
            valid = false;
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

        Playlist(){
            use_file = "playlist.pls";
        }

    } p;

    struct ProgressBar : public Valid
    {
    public:

        unsigned int current_cursor = 0;

        void refresh_mode_progress(){
            auto d = audio::position::get_d();
            auto s = element.space;
            auto idx = static_cast<unsigned int>(s.w * d);
            current_cursor = idx;
            mv(s.x, s.y);
            
            use_attr( attr_reset << color_color(color_played));
            for (size_t i = 0; i < s.width(); i++)
            {
                if (i == idx)
                    std::cout << attr_reset << color_color(color_cursor) << char_cursor << color_color(color_remaining);
                else if (i == 0)
                    std::cout << char_first;
                else if (i == static_cast<unsigned long>(s.width() - 1))
                    std::cout << char_last;
                else
                    std::cout << char_center;
            }
            use_attr(attr_reset)
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

        bool valid_mode_progress(){
            auto d = audio::position::get_d();
            auto s = element.space;
            auto idx = static_cast<unsigned int>(s.w * d);
            return current_cursor == idx;
        }
        bool valid_mode_command(){
            return false;
        }
        bool valid_mode_search(){
            return false;
        }
        bool valid_mode_unknown(){
            return false;
        }


        StringLite char_first = "├";
        StringLite char_last = "┤";
        StringLite char_center = "─";
        StringLite char_cursor = "•";

        Color color_played = {default_warn_fg, default_warn_bg, ""};
        Color color_remaining  = {default_normal_fg, default_normal_bg, ""};
        Color color_cursor = {{255,0,255,}, {0,0,0}, ""};

        bool valid = false;

        wm::Element element;


        void action(wm::MOUSE_INPUT m){//do something with the input

        };
        bool is_valid() override{
            if(!valid)
                return false;

                //valid functions
            switch (input_mode.num) {
                case InputMode::PROGRESS: return valid_mode_progress();
                case InputMode::COMMAD: return valid_mode_command();
                case InputMode::SEARCH: return valid_mode_search();
                default: return valid_mode_unknown();
            }

        }
        void refresh() override{
            valid = true;
            clog << "ProgressBar refrehs "<< input_mode.num << std::endl;

            switch (input_mode.num) {
                case InputMode::PROGRESS:
                    refresh_mode_progress();
                    break;
                case InputMode::COMMAD:
                    refresh_mode_command();
                    break;
                case InputMode::SEARCH:
                    refresh_mode_search();
                    break;
                default:
                    refresh_mode_unknown();
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

    StringLite CoverArtData::placeholder_path = "gluttony.png";
    StringLite CoverArtData::cache_path = "tmp.png";

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
            use_attr(attr_reset)
        };


        inline void draw_image(){
            if(!std::filesystem::exists(data.file_path.get_p()))
                return;

            auto s = element.wSpace();
            std::string fpath = data.file_path.get_p();
            ascii_img::load_image_t *cover_ansi = audio::extra::getImg(fpath, s.width(), s.height() + 1);
            
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
            std::thread* thr = data.img_valid ? nullptr : new std::thread(fetch_coverart, &data); 
            valid = true;
            data.img_valid = true;
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
        struct Prev: public ButtonArrayElement{
            //extra shit
            static StringLite ch;
            bool valid = false;

        } prev_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr( attr_reset );
                if(inside){
                    use_attr(color_color(colors.hover)); 
                    if(m.btn == wm::MOUSE_BTN::M_LEFT)
                        load::prev();
                }
                else
                    use_attr(color_color(colors.normal))
                
                std::cout << prev_bae.ch << ' ' << attr_reset;
                prev_bae.valid = true;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::PREV,
            []{return prev_bae.valid;},
            []{prev_bae.valid = false;}

        };

        struct Next : public ButtonArrayElement {
            static StringLite ch;
            bool valid = false;
        } next_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT)
                        load::next();
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << next_bae.ch << ' ' << attr_reset;
                //validate
                next_bae.valid = true;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::NEXT,
            []{return next_bae.valid;},
            []{next_bae.valid = false;}
        };

        

        struct PlayPause: public ButtonArrayElement
        {
            static StringLite playing_ch;
            static StringLite stopped_ch;
            enum State : unsigned char{
                INVALID,
                PLAYING,
                STOPPED,
            } state;
            /* data */
        } playpause_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                auto& usech = audio::playing() ? playpause_bae.playing_ch : playpause_bae.stopped_ch;
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT && p.size())
                        control::playpause();
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << usech << ' ' << attr_reset;
                playpause_bae.state = audio::playing() ? PlayPause::PLAYING:PlayPause::STOPPED;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::PLAYPAUSE,
            []{
                switch (playpause_bae.state)
                {
                case PlayPause::State::INVALID:
                    return false;
                case PlayPause::State::PLAYING:
                    return audio::playing();
                case PlayPause::State::STOPPED:
                    return !audio::playing();
                default:
                    playpause_bae.state = PlayPause::State::INVALID;
                    break;
                }
                return false;
            },
            []{playpause_bae.state = PlayPause::State::INVALID;}
        };



       


        struct Shuffle: public ButtonArrayElement {
            static StringLite shuffle_ch;
            static StringLite sort_ch;
            enum State : unsigned char {
                INVALID,
                SORTED,
                SHUFFLED,
            } state = State::INVALID;
            

        } shuffle_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                auto& usech = p.sorted() ? shuffle_bae.sort_ch : shuffle_bae.shuffle_ch;
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
                shuffle_bae.state = p.sorted() ? Shuffle::SORTED : Shuffle::SHUFFLED;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::SHUFFLE,
            [](){return shuffle_bae.state == (p.sorted() ? Shuffle::SORTED : Shuffle::SHUFFLED);},
            [](){shuffle_bae.state = Shuffle::INVALID;},
        };

        struct Loop : ButtonArrayElement {
            static StringLite track_ch;
            static StringLite playlist_ch;
            static StringLite off_ch;
            enum State : unsigned char {
                INVALID,
                OFF,
                PLAYLIST,
                TRACK,
            } state = INVALID;
        } loop_bae = {
        [](bool inside, wm::MOUSE_INPUT m)
            {
                use_attr(attr_reset);
                StringLite &use_char = p.loopType == p.L_NONE ? loop_bae.off_ch : p.loopType == p.L_PLAYLIST? loop_bae.playlist_ch : loop_bae.track_ch;
                
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
                loop_bae.state =  p.loopType == p.L_NONE ? Loop::OFF : p.loopType == p.L_PLAYLIST? Loop::PLAYLIST : Loop::TRACK;
            },
            2,
            GROUPS::MEDIA_CONTROLS,
            IDS::LOOP,
            [](){
                switch (loop_bae.state)
                {
                case Loop::TRACK:
                    return p.loopType == Playlist::LoopType::L_TRACK;
                case Loop::PLAYLIST:
                    return p.loopType == Playlist::LoopType::L_PLAYLIST;
                case Loop::OFF:
                    return p.loopType == Playlist::LoopType::L_NONE;
                case Loop::INVALID:
                default:
                    break;
                }
                return false;
            },
            [](){loop_bae.state = Loop::State::INVALID;}
        };

        StringLite Shuffle::shuffle_ch = "⤮";
        StringLite Shuffle::sort_ch = "⇉";

        StringLite Prev::ch = "⏮";
        StringLite Next::ch = "⏭";

        StringLite PlayPause::playing_ch = "▶";
        StringLite PlayPause::stopped_ch = "⏸";

        StringLite Loop::track_ch = "⥀";
        StringLite Loop::playlist_ch = "♺";
        StringLite Loop::off_ch = "🡢";

        struct Volume : public ButtonArrayElement {
            int vol = 0;
            bool valid = false;
        } volume_bae = {
    [](bool inside, wm::MOUSE_INPUT m)
        {
            use_attr(attr_reset);
            // functionality
            if (inside)
            {
                use_attr(color_color(colors.hover));

                if (m.btn == wm::MOUSE_BTN::M_SCRL_UP)
                {
                    control::volume_rel(volume_shift);
                }
                else if (m.btn == wm::MOUSE_BTN::M_SCRL_DOWN)
                {
                    control::volume_rel(-volume_shift);
                }
                else if (m.btn == wm::MOUSE_BTN::M_LEFT)
                {
                    control::volume_set(volume_reset);
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
        IDS::VOLUME,
        []{
            return volume_bae.valid || std::abs( (int) std::round(audio::volume::get()) ) == volume_bae.vol;
            },
        []{volume_bae.valid = false;},

        };

        #define leading_zero(n) ((n < 10) ? "0" : "") << n
        struct Time : ButtonArrayElement {
            long long current_time = 0;
            bool valid = false;
        } time_bae = {
            [](bool inside, wm::MOUSE_INPUT m){
                auto ps = audio::position::get<std::ratio<1, 1>>().count();
                auto ts = audio::duration::get<std::ratio<1, 1>>().count();

                auto pm = ps / 60;
                auto tm = ts / 60;

                auto psec = ps - pm * 60;
                auto tsec = ts - tm * 60;

                use_attr(attr_reset)
                if(inside)
                    use_attr(color_color(colors.hover))
                else
                    use_attr(color_color(colors.normal))


                std::cout 
                << leading_zero(pm) << ':' << leading_zero(psec)
                << '/'
                << leading_zero(tm) << ':' << leading_zero(tsec)
                << attr_reset;
            },
            11,
            GROUPS::TIME,
            IDS::TIME,
            []{
                return time_bae.valid || time_bae.current_time == audio::position::get<std::ratio<1, 1>>().count();
            },
            []{time_bae.valid = false;},
        };




    } // namespace ControlButtons
    
    static std::vector<ButtonArrayElement> controlButtons({
        ControlButtons::prev_bae,
        ControlButtons::playpause_bae,
        ControlButtons::next_bae,
        ControlButtons::shuffle_bae,
        ControlButtons::loop_bae,
        ControlButtons::volume_bae,
        ControlButtons::time_bae,
    });

    class Controls :public ButtonArray, public Valid{
    public:

        void action(wm::MOUSE_INPUT m){
            draw(m);
        };
        //why? idk man...
        void invalidate_all(){
            for (auto e : *this) {
                if(e.invalidate)
                    e.invalidate();
            }
        }

        bool is_valid() override{
            auto ret = true;
            for (auto e : *this) {
                if(!e.is_valid()){
                    clog << "Controls:IsValid: "<<e.is_valid() << " Alloc: " <<e.alloc<< " Group:" << e.group << " ID: " <<e.id << std::endl; 
                    ret =  false;
                }

            }
            return ret;
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
            clog << "Refresh Title" << std::endl;
            valid = true;
            if(p.empty())
                return;
            //set window title
            //maby set metadata also but well do it later or something :3
            set_title((window_title_filename_only ? path::filename(p.current()) : p.current()).c_str());
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
                p.element.space.x,
                p.element.space.end().y, 
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
            if(!p.is_valid()){
                clog << "Playlist Refresh" << std::endl;
                p.refresh();
            }
        };
        inline void progressbar(){
            if(!progres_bar.is_valid()){
                clog << "Progressbar Refresh" << std::endl;
                progres_bar.refresh();
            }
        };
        inline void coverart(){
            if(!cover_art.is_valid()){
                clog << "CoverArt Refresh" << std::endl;
                cover_art.refresh();
            }
        };
        inline void config(){
            if(!cfg.is_valid()){
                clog << "Config Refresh" << std::endl;
                cfg.refresh();
            }
        };
        //todo
        inline void controls(){
            if(!ctrl.is_valid()){
                clog << "Controls Refresh" << std::endl;
                ctrl.draw({mpos, wm::MOUSE_BTN::M_NONE, true});
            }
        };
        inline void r_title(){
            if(p.empty() || title.valid)
                return;
            title.refresh();
            clog << "Title Refresh" << std::endl;
        };

        inline void all(){
            if(wm::resize_event){
                wm::resize_event = false;
                clog << (display_mode.horizontal() ? "HORIZONTAL " : "VERTICAL ") << wm::WIDTH << '/' << wm::HEIGHT << std::endl; 
                clog << "RESIZE EVENT" << std::flush;
                p.valid = false;
                progres_bar.valid = false;
                cover_art.valid = false;
                title.valid = false;
                for(auto& e : ctrl){
                    if(e.invalidate)
                        e.invalidate();
                }
                clear_scr();
            }
            elements();
            playlist();
            progressbar();
            controls();
            r_title();
            coverart();
            std::cout.flush();
        };

    } // namespace refresh

    namespace load
    {
        inline void file(const std::string& filepath){
            loading = true;
            clog << filepath << std::endl;
            if(!filepath.size() || !std::filesystem::exists(filepath)){
                loading = false;return;
            }
            //invalidate shit and stuff
            audio::load(filepath);
            control::play();
            p.valid = false;
            cover_art.valid = false;
            cover_art.data.img_valid = false;
            ctrl.invalidateGroup(GROUPS::TIME);
            ctrl.invalidateGroup(GROUPS::MEDIA_CONTROLS);
            title.valid = false;
            loading = false;
        }

        inline void next(){
            if(loading)
                return;
            loading = true;
            auto o = p.opt_next();
            if(!o.has_value()){
                clog << color_color(colors.warn)<<"!FAILED TO CHANGE SONG!" << attr_reset;
                return;
            }
            file(*o);

            
        }
        inline void prev(){
            if(loading)
                return;
            loading = true;
            auto o = p.opt_prev();
            if(!o.has_value()){
                clog << color_color(colors.warn) << "!FAILED TO CHANGE SONG!" << attr_reset;
                return;
            }
            file(*o);

        }
        inline void curr(){
            if(loading)
                return;
            loading = true;
            auto o = p.opt_curr();
            if(!o.has_value()){
                clog << color_color(colors.warn)<<"!FAILED TO CHANGE SONG!" << attr_reset;
                return;
            }
            file(*o);

        }

    } // namespace load

    namespace control
    {
        inline void playpause(){
            if(p.empty())
                return;
            
            audio::control::toggle();
            #if defined(MPRIS)
            s.set_playback_status(audio::playing()? mpris::PlaybackStatus::Playing : mpris::PlaybackStatus::Paused );
            #endif // MPRIS
            ctrl.invalidateId(IDS::PLAYPAUSE);
        }
        inline void play(){
            if(p.empty())
                return;

            audio::control::play();
            #if defined(MPRIS)
            s.set_playback_status(mpris::PlaybackStatus::Playing);
            #endif // MPRIS
            
            ctrl.invalidateId(IDS::PLAYPAUSE);
        }
        inline void pause(){
            if(p.empty())
                return;

            audio::control::pause();
            #if defined(MPRIS)
            s.set_playback_status(mpris::PlaybackStatus::Paused);
            #endif // MPRIS
            ctrl.invalidateId(IDS::PLAYPAUSE);
        };
        inline void shuffle(){
            if(p.empty())
                return;
            if(!p.seed)
                p.new_seed();
            p.shuffle();
            ctrl.invalidateId(IDS::SHUFFLE);
            p.valid = false;
        }
        inline void sort(){
            if(p.empty())
                return;
            p.sort();
            ctrl.invalidateId(IDS::SHUFFLE);
            p.valid = false;

        }

        inline void volume_set(double d){
            audio::volume::set(d);
            #if defined(MPRIS)
            s.set_volume(audio::volume::get());
            #endif // MPRIS
        }
        inline void volume_rel(double d){
            audio::volume::shift(d);
            #if defined(MPRIS)
            s.set_volume(audio::volume::get());
            #endif // MPRIS
        }

        inline void volume_set_invalidate(double d){
            volume_set(d);
            ctrl.invalidateId(IDS::VOLUME);
        }
        inline void volume_rel_invalidate(double d){
            volume_rel(d);
            ctrl.invalidateId(IDS::VOLUME);
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

    
    inline void at_quit(){
        exit(0);
    }

    #if defined(MPRIS)
    void mpris_init(){


        s.set_identity("Terminal Musicplayer");
        s.on_quit([](){
            clog << "QUIT" << std::endl;
        });


        s.on_seek([&] (int64_t p) {
            clog << "s.on_seek:p:" << p <<std::endl;
            audio::seek::rel(std::chrono::seconds(p));
            auto t = audio::position::get<std::ratio<1>>();
            s.set_position(t.count());
        });
        s.on_set_position([&] (int64_t p) {
            clog << "s.on_set_position:p:" << p <<std::endl;
            audio::seek::rel(std::chrono::seconds(p));
            auto t = audio::position::get<std::ratio<1>>();
            s.set_position(t.count());
        });

        s.on_next([&](){
            clog << "on_next:" << std::endl;
            load::next();
        });
        s.on_previous([](){
            load::prev();
        });
        s.on_play([](){
            control::play();
        });
        s.on_pause([](){
            control::pause();
        });
        s.on_stop([]{
            control::pause();
        });
        s.on_play_pause([&](){
            clog << "MPRIS PLAYPAUSE" << std::endl;
            control::playpause();
        });

        s.on_loop_status_changed([&] (mpris::LoopStatus status) {
            switch (status)
            {
            case (mpris::LoopStatus::None):
                p.loopType = Playlist::LoopType::L_NONE;
                break;
            case (mpris::LoopStatus::Playlist):
                p.loopType = Playlist::LoopType::L_PLAYLIST;
                break;
            case (mpris::LoopStatus::Track):
                p.loopType = Playlist::LoopType::L_TRACK;
                break;
            default:
                p.loopType = Playlist::LoopType::L_PLAYLIST;
                break;
            }
            ctrl.invalidateId(IDS::LOOP);
        });
        s.on_shuffle_changed([&] (bool shuffle) {
            if(shuffle)
                control::shuffle();
            else
                control::sort();
        });
        s.on_volume_changed([&] (double vol) {
            control::volume_set(vol);
            ctrl.invalidateId(IDS::VOLUME);
        });
        
        s.set_volume(audio::volume::get());

        s.start_loop_async();
        clog << "Mpris Initialized" << std::endl;
    }
    #endif // MPRIS

    std::chrono::milliseconds d(static_cast<long>((1./24.)*1000));
    std::thread* draw_thread;
    inline void draw_thr_fn(){
        while (draw)
        {
            if(audio::stopped() && !load::loading){
                if(p.loopType == p.L_TRACK)
                    load::curr();
                else
                    load::next();
            }
            #if defined(MPRIS)
            {
                auto t = audio::position::get<std::ratio<1>>();
                s.set_position(t.count());
            }
            
            #endif // MPRIS
            
            //std::lock_guard lock(drawing);

            refresh::all();
            
            std::this_thread::sleep_for(d);
        }
    }

    

    inline void init(int argc,  char** const argv){

        clear_all_no_mouse();
        use_attr(alt_buffer);

        parse_arguments(argc, argv);
        std::cerr.rdbuf(clog.rdbuf());

        
        //atexit(at_exit);
        //signal(SIGINT, at_int);

        wm::init();
        use_attr(cursor_invisible);

        cfg.configuration();
        cfg.refresh();

        auto seek_to = p.use();
        if(p.size()){
            if(p.seed)
                control::shuffle();
            
            load::curr();
        }
        p.cursor_offset = p.current_index;
        p.display_offset = p.current_index;
        audio::seek::abs(std::chrono::seconds(seek_to));

        draw_thread = new std::thread(draw_thr_fn);

        #if defined(MPRIS)
        
        mpris_init();
        
        #endif // MPRIS
        

    }

    inline void deinit(){
    }

} // namespace temqo

