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
#include "lib/wm/getch.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/init.hpp"
#include <algorithm>
#include <bits/getopt_core.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <ratio>
#include <sdbus-c++/Error.h>
#include <sdbus-c++/Types.h>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <signal.h>
#include <variant>
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


//TODO, Add option to swicth and discern different subtitles
#define CAPTIONS 1
#if defined(CAPTIONS)
#include "custom/lyrics.hpp"
#endif // CAPTIONS


#define clip_n(var, min, max) if(var > max){var = max;}if(var < min){var = max;}

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

    enum InputMode : unsigned char{
        COMMON, COMMAND, SEARCH
    } input_mode = COMMON;
    //input buffer;
    class Input
    {
    private:
        /* data */
    public:
        void clear(){
            input.clear();
            search_offset  = 0;
            cursor_offset  = 0;
        }
        std::string str(){
            return input;
        }
        int search_offset = 0;
        int cursor_offset = 0; //cursor [0, input.size()] // so it can be equal to input.size()
        std::string input;
        std::string render(){
            std::string out;
            out.reserve(input.size()+1 + (4+5));
            for (size_t i = 0; i < input.size(); i++)
            {
                if(i == cursor_offset){
                    out+= underline;
                    out+= input[i];
                    out+= underline_reset;
                }
                else{
                    out+=input[i];
                }
            }
            if(cursor_offset == input.size()){ //add the cursor to the end
                out+= underline;
                out+= ' ';
                out+= underline_reset;
            }

            return out;
        }

        inline Input& shift_left(){
            if(cursor_offset > 0){
                cursor_offset--;
            }
            return *this;
        }
        inline Input& shift_right(){
            if(cursor_offset < input.size() && input.size() != 0){
                cursor_offset++;
            }
            return *this;
        }

        //MARKER
            //like DEL key (not backspace)
        inline Input& delete_front(){
            if(cursor_offset < input.size()-1  && input.size() != 0){
                auto it = input.begin();
                std::advance(it, cursor_offset);
                input.erase(it);
            }
            return *this;
        }
            //Like backspace
        inline Input& delete_behind(){
            if(cursor_offset > 0 ){
                cursor_offset--;
                auto it = input.begin();
                std::advance(it, cursor_offset);
                input.erase(it);
            }
            return *this;
        }

        inline void setCursor(size_t offset){
            if(offset >= 0 && offset < input.size()){
                cursor_offset = 0;
            }
        }

        inline Input& append(char c){
            auto it = input.begin();
            std::advance(it, cursor_offset);
            input.insert(it, c);
            cursor_offset++;
            return *this;
        }

        inline Input& operator+=(char c){
            return append(c);
        }
        Input(/* args */) {}
        ~Input() {}
    } input;
    



    static float volume_shift = 5;
    static float volume_reset = 100;
    static double playlist_coverart_ratio = 0.5;
    bool elements_refresh = true;
    wm::Position mpos;

    class Valid
    {
    public:
        virtual void refresh(){};
        virtual void invalidate(){};
        virtual bool is_valid(){return true;};
    };

    class Action{
    public:
        virtual void m_action(wm::MOUSE_INPUT m) {};
        virtual void k_action(wm::KEY k) {};
        virtual void ch_action(int c) {};
    };

    #if defined(MPRIS)
    mpris::Server s("Temqo");
    #endif // MPRIS

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
        inline void seek_abs(double);
        template<typename T, typename Rat>
        inline void seek_rel(std::chrono::duration<T, Rat> d);
        inline void volume_set(double);
        inline void volume_rel(double);
        inline void volume_set_invalidate(double);
        inline void volume_rel_invalidate(double);
    } // namespace control

    namespace refresh
    {
        inline unsigned int get_useable_height();
        inline unsigned int get_useable_width();
    } // namespace refresh
    

    enum DisplayFileMode: unsigned char {
        FILENAME,
        FILE,
        FILEPATH,
        METADATA_TITLE,
    };
    std::string DisplayFileParse(const std::string& str, DisplayFileMode mode,  const std::optional<audio::extra::AudioMetadata>& o_md = std::nullopt){
        switch (mode) {
            case DisplayFileMode::METADATA_TITLE:
                if(o_md.has_value())
                {
                    if(o_md->title.lenght()){
                        return o_md->title.get_p();
                    }
                }
            case DisplayFileMode::FILENAME:
                return path::filebasename(str);
            case DisplayFileMode::FILE:
                return path::filename(str);
            case DisplayFileMode::FILEPATH:
                return str;
            default:
                return path::filebasename(str);
        }
    }
    
    

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
        static StringLite path;
        bool valid = false;

        void configuration(){//do the config
            cfg::il_cfg.clear();
            cfg::ml_cfg.clear();
            //do the configs again
        }

        bool is_valid() override{ //maby calculate file hash??
            return valid;
        }
        void refresh() override{
            valid = true;
            cfg::parse(path);
        }
        void invalidate() override{
            valid = false;
        }

        Config(){configuration();}
        Config(const char* ch){
            Config::path = ch;
            configuration();
        }
    } cfg("./temqo.cfg");
    StringLite Config::path = "./temqo.cfg";
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

    class Playlist : public audio::Playlist, public Valid, public Action{
    public:
        static wm::SPLICE_TYPE clip_type;
        static wm::PAD_TYPE pad_type;

        size_t cursor_offset = 0;
        size_t display_offset = 0;
        wm::Element element;
        
        bool valid = false;

        void invalidate() override{
            valid = false;
        }

        DisplayFileMode dplfile = DisplayFileMode::FILEPATH;

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
            std::string pstr;
            pstr.append(ws.w+1, ' ');
            clog << "ws: " << ws << "W/H" << wm::WIDTH << '/'  << wm::HEIGHT << std::endl;
            for (size_t index = 0; index < ws.h; index++)
            {
                auto i = clamp(display_offset + index);
                auto o = opt_get(i);

                if(!o.has_value())
                    continue;
                    
                std::string str = DisplayFileParse(*o, dplfile, dplfile==METADATA_TITLE ? audio::extra::getMetadata(*o) : std::nullopt);
                

                auto i_str = std::to_string(i) + ' ';
                wm::clip(str, ws.width() - i_str.length()+1, clip_type);
                //wm::pad(str, ws.width() - i_str.length(), pad_type);
                str = i_str + str;

                if (i == static_cast<unsigned int>(cursor_offset))
                    use_attr(color_color(colors.hover));
                if (i == current_index)
                    use_attr(color_color(colors.hilight));

                auto mvstr =  mv_str(ws.x, ws.y + index);
                std::cout << mvstr << pstr << mvstr << str << attr_reset;
            }
            //clog << "PLAYLIST_LAST: " <<  (*this)[ clamp(display_offset + ws.h-1)];

        }
        size_t st_index()
        {
            return clamp(display_offset);
        }
        size_t last_index()
        {
            return clamp(display_offset + element.wSpace().h-1);
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

                //implement and replace the mouse scrl w this
        //void shift_up(size_t n = 1){
        //    
        //}
        void curs_up(size_t n = 1){
            cursor_offset-=n;
            ref_disp_offset();
            invalidate();
        }
        void curs_down(size_t n = 1){
            cursor_offset+=n;
            ref_disp_offset();
            invalidate();
        }
        //bool inside = false;
        char scroll_sensitivity = 1;
        void m_action(wm::MOUSE_INPUT m) override {
            //handle mouse action
            //clog << m << std::endl;
            if(element.wSpace().inside(m.pos)){
                //inside= true;
                //do the magic
                if(m.btn == wm::MOUSE_BTN::M_SCRL_UP){
                    curs_up(scroll_sensitivity);
                }else if(m.btn == wm::MOUSE_BTN::M_SCRL_DOWN){
                    curs_down(scroll_sensitivity);
                }else{
                    auto s = element.wSpace();
                    auto i = m.pos.y - s.y;
                    cursor_offset = clamp(display_offset + i);
                    ref_disp_offset();
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        if (element.aSpace().width() < 5 || element.aSpace().height() < 5){}
                        else{
                            play_current_cursor();
                        }
                    }
                }
                invalidate();
            }
            //else /*if(inside)*/{
            //    //inside = false;
            //    //do you need to refresh if its not inside??
            //    //invalidate();
            //}
        }
        void k_action(wm::KEY k) override {
            //handle arrow key action
            if(input_mode != InputMode::COMMON)
                return;
            if(k == wm::K_DOWN){
                curs_down();
            }else if(k == wm::K_UP){
                curs_up();
            }
        }
        void play_current_cursor(){
            if(!empty() && cursor_offset < size()){
                current_index = cursor_offset;
                load::curr();
            }
        }
        void ch_action(int c) override {
            //handle char action
            //it doesnt need charachter action... i think
            if(input_mode == InputMode::COMMON){
                if(c == '\n'){
                    play_current_cursor();
                }
            }
        }

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


    #if defined(MPRIS)
    #define mpris_setstr(property, mpfield)  if(md->property.length()) set(mpris::Field::mpfield, std::string(md->property.get_p()));
    #define mpris_seti(property, mpfield)  if(md->property) set(mpris::Field::mpfield, static_cast<int>(md->property));

    

    struct Metadata:  public std::map<mpris::Field, sdbus::Variant>, public Valid{
        bool valid = false;
        bool is_valid() override{
            return valid;
        }
        void refresh() override{
            auto o = p.opt_curr();
            if(o.has_value()){
                auto md = audio::extra::getMetadata(*o);
                if(md.has_value()){ //set extra metadata
                    mpris_setstr(album,     Album)
                    mpris_setstr(title,     Title)
                    mpris_setstr(artist,    Artist)
                    mpris_setstr(genre,     Genre)
                    mpris_setstr(comment,   Comment)
                    mpris_seti  (track,     TrackNumber)
                }
            }

            s.set_metadata(*this);
            valid = true;
        }
        void invalidate() override{
            valid = false;
        }
        //bool action() override{}


        ///invalidates the metadata for refreshing
        void set(const key_type& k, const mapped_type& m){
            this->insert_or_assign(k, m);
            valid = false;
        }
        //sdbus::Variant& operator[](const key_type& k){
        //    return this->at(k);
        //}
    } metadata;
    
    #endif // MPRIS
    
    wm::SPLICE_TYPE Playlist::clip_type = wm::SPLICE_TYPE::BEGIN_CUT;
    wm::PAD_TYPE Playlist::pad_type = wm::PAD_TYPE::PAD_RIGHT;
    //manages the I/O
    struct ProgressBar : public Valid, public Action
    {
    public:

        unsigned int current_cursor = 0;
        unsigned skips = 0;
        int displayed_skip = 0;
        StringLite current_input = "";
        InputMode current_drawn_mode = static_cast<InputMode>(-1);

        StringLite char_first = "├";
        StringLite char_last = "┤";
        StringLite char_center = "─";
        StringLite char_cursor = "•";

        Color color_played = {default_warn_fg, default_warn_bg, ""};
        Color color_remaining  = {default_normal_fg, default_normal_bg, ""};
        Color color_cursor = {{255,0,255,}, {0,0,0}, ""};

        struct FoundThings : std::vector<size_t>
        {
            /* data */
            size_t& get(size_t index){
                return at(index%size());
            }
        }  found_things;

        wm::Element element;
        bool valid = false;
        //enum FindType : unsigned char{
        //    MULTIWORD,
        //    STRING_INSENSITIVE,
        //} findtype = MULTIWORD;


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

        void ref_str(std::string& str, size_t width){
            current_input = input.str();
            current_cursor = input.cursor_offset;
            //wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
            clog << "ProgressBar: " << width << " string lenght: " << str.length() << std::endl;
            for (size_t i = 0; i < width; i++){
                    //cursor is fucked up
                    //maby off set it somehow?
                        //idk man 2 lazy
                if(i < str.size()){
                    std::cout << (i == input.cursor_offset ? underline : "") << str[i] << (i == input.cursor_offset ? underline_reset : "");
                }else{
                    std::cout << (i == input.cursor_offset ? underline : "") << ' ' << (i == input.cursor_offset ? underline_reset : "");
                }
            }
        }

        void refresh_mode_command(){
            auto s = element.space;
            std::string str = input.str();
            wm::clip(str, s.width() -1 , wm::SPLICE_TYPE::BEGIN_DOTS);
            mv(s.x, s.y);
            std::cout << ':';
            ref_str(str, s.width()-1);
        }

        void refresh_mode_search(){
            auto s = element.space;
            std::string str = input.str();
            mv(s.x, s.y);
            displayed_skip = skips;
            std::string prefix = "(" + std::to_string(found_things.size() ? skips+1 : 0) + "/" + std::to_string(found_things.size()) + ")"+"/";
            std::cout << prefix;
            auto w = s.width() - prefix.size();
            wm::clip(str, w , wm::SPLICE_TYPE::BEGIN_DOTS);
            ref_str(str, w);
        }

        void refresh_mode_unknown(){
            auto s = element.space;
            std::string str = input.str();
            wm::clip(str, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
            mv(s.x, s.y);
            ref_str(str, s.width());
        }

        bool valid_mode_progress(){
            auto d = audio::position::get_d();
            auto s = element.space;
            auto idx = static_cast<unsigned int>(s.w * d);
            return current_cursor == idx;
        }
        bool valid_mode_command(){
            return current_input.get_p() == input.str() && current_cursor == input.cursor_offset;
        }
        bool valid_mode_search(){
            return current_input.get_p() == input.str() && current_cursor == input.cursor_offset && displayed_skip == skips;
        }
        bool valid_mode_unknown(){
            return true;
        }

        //MARKER
        void m_action(wm::MOUSE_INPUT m) override {
            //handle mouse action
            if(input_mode == InputMode::COMMON){
                auto s = element.aSpace();
                if(s.inside(m.pos)){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        auto delta = static_cast<double>(m.pos.x) / static_cast<double>(s.width());
                        control::seek_abs(delta);
                    }
                }
            }
            else if(input_mode == InputMode::COMMAND || input_mode == InputMode::SEARCH){
                //yea just bullshit
                auto s = element.aSpace();
                if(s.inside(m.pos)){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT && s.x >= m.pos.x+1){
                        size_t offset = s.x - m.pos.x+1;
                        input.setCursor(offset);
                    }
                }
            }

        }
        

        void refresh_curser(){
            if (found_things.empty()){
                clog << "Found was empty" << std::endl;
                return;
            }
            auto& use = found_things.get(skips);
            clog << "index: " << use << std::endl;
            p.cursor_offset = p.clamp(use);
            p.ref_disp_offset();
            p.invalidate();
        }
        void refresh_search(){
            found_things.clear();
            p.find2allmulti(found_things, input.str());
            refresh_curser();
        }
        
        void k_action(wm::KEY k) override {
            //handle arrow key action
            if(input_mode == InputMode::COMMAND || input_mode == InputMode::SEARCH){
                if(k == wm::KEY::K_LEFT)
                    input.shift_left();
                else if(k == wm::KEY::K_RIGHT)
                    input.shift_right();
            }
            if(input_mode == InputMode::SEARCH){
                if(k == wm::KEY::K_DOWN){
                    if(!found_things.empty()){
                        skips++;
                        if(skips >= found_things.size()){
                            skips = 0;
                        }
                        refresh_curser();
                        invalidate();
                        p.invalidate();
                    }
                }
                else if(k == wm::KEY::K_UP){
                    if(!found_things.empty()){
                        if(skips == 0){
                            skips = found_things.size();
                        }
                        skips--;
                        refresh_curser();
                        invalidate();
                        p.invalidate();
                    }
                }
            }

        }

        void ch_action(int c) override {
            //handle char action
            if(input_mode == InputMode::COMMAND){
                //yea just bullshit
                if(c == 127){
                    input.delete_behind();
                }else if(c == '\n'){
                    input.clear();
                    input_mode = InputMode::COMMON;
                    clog << "SET TO COMMON FROM NEWLINE CHARACHER" << std::endl;
                }else{
                    input+=c;
                }
            }
            else if (input_mode == InputMode::SEARCH) {
                skips = 0; // fuck them kids
                if(c == '\n'){
                    input.clear();
                    input_mode = InputMode::COMMON;
                    clog << "SET TO COMMON FROM NEWLINE CHARACHER" << std::endl;
                    p.play_current_cursor();
                    return;
                }
                if(c == 127){
                    input.delete_behind();
                }else{
                    input+=c;
                }
                refresh_search();
            }

        }

        bool is_valid() override{
            if(!valid || current_drawn_mode != input_mode)
                return false;

                //valid functions
            switch (input_mode) {
                case InputMode::COMMON: return valid_mode_progress();
                case InputMode::COMMAND: return valid_mode_command();
                case InputMode::SEARCH: return valid_mode_search();
                default: return valid_mode_unknown();
            }

        }
        void refresh() override{
            valid = true;
            current_drawn_mode = input_mode;
            switch (input_mode) {
                case InputMode::COMMON:
                    refresh_mode_progress();
                    break;
                case InputMode::COMMAND:
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

    StringLite CoverArtData::placeholder_path = "phrogt.jpg";
    StringLite CoverArtData::cache_path = "tmp.png";

    inline void fetch_coverart(CoverArtData* ptr){
        ptr->img_valid = true;
        auto o = p.opt_curr();
        if(ptr->override || p.empty() || !o.has_value()){
            ptr->file_path = ptr->placeholder_path.get_p();
            //clog << "p.empty()        "<< (p.empty()        ? "true" : "false") << std::endl;
            //clog << "ptr->override    "<< (ptr->override    ? "true" : "false") << std::endl;
            //clog << "!o.has_value()   "<< (!o.has_value()   ? "true" : "false") << std::endl;
            //clog<< "using placeholder: " << ptr->file_path << std::endl;
        }
        else{
            auto res = audio::extra::extractAlbumCoverTo(*o, ptr->cache_path);
            ptr->file_path = ((res == 0) ? ptr->cache_path : ptr->placeholder_path).get_p();
        }
            //we need the absolute path
        std::string fpath = "file:///"+std::filesystem::absolute(ptr->file_path.get_p()).generic_string();
        clog << "fetch_coverart: fpath: " << fpath << std::endl;
        #if defined(MPRIS)
        metadata.set(mpris::Field::ArtUrl, fpath);
        #endif
    }
    
    #if defined(CAPTIONS)
    
    class Lyrics: public Valid, public Action
    {
    private:
        
    public:
        wm::Element element;
        std::vector<Lyric> l;
        std::string current_song;
        size_t current_lyric;
        bool valid = false;
        //in seconds
        size_t get_valid_index(double time){
            for (auto i = 0; i< l.size();i++) //linear bcs ez
            {
                auto& y = l[i];
                if(y.end_time > time){
                    return i;
                }
            }
            return std::variant_npos;
        }

        bool song_valid(){
            if(p.current() == current_song)
                return true;
            return false;
        }

        bool time_valid(){
            if(l.size() > current_lyric){
                auto& y  = l[current_lyric];
                if(audio::position::get<std::milli>().count() >= y.end_time){
                    //invalid
                    //clog << "Lyrics were invalid by time: " << audio::position::get<std::milli>().count() << ">=" << y.end_time << std::endl;
                    return false;
                }
            }
            return true;
        }
        void refresh_song(){
            l.clear();
            current_song = p.current();
            auto path = scan_lyrics(current_song);
            if(!path.empty()){
                clog << "Found: " << path;
                l.clear();
                parse_lyrics(path, l);
            }

        }

        void clear(){
            auto s = element.wSpace();
            auto mvstr = mv_str(s.x, s.y);
            std::string pad;
            pad.append(s.w, ' ');
            std::cout << mvstr << pad;
        }

        void refresh() override{
            //clog << "Lyrics Refresh" << std::endl;
            if(!song_valid())
                refresh_song();
            
            valid = true;
            current_lyric = get_valid_index(audio::position::get<std::milli>().count());
            if(current_lyric == std::variant_npos){
                clog << "Npos" << std::endl;
                clear();
                return;
            }
            if(current_lyric >= l.size()){
                clog << "Bigger than size" << std::endl;
                clear();
                return;
            }
            auto y = l[current_lyric];
            auto s = element.wSpace();
            wm::clip(y.str, s.width(), wm::SPLICE_TYPE::END_CUT);
            wm::pad(y.str, s.width(), wm::PAD_TYPE::PAD_CENTER);
            auto mvstr = mv_str(s.x, s.y);
            std::string pad;
            pad.append(s.w, ' ');
            clog << y.str << std::endl;
            std::cout << mvstr << pad << mvstr << y.str;
        }

        void invalidate() override{
            valid = false;
        }

        bool is_valid() override{
            //clog << "ISVALID CALLED" << std::endl;
            return valid && time_valid() && song_valid();
        }

        


        Lyrics(/* args */) {}
        ~Lyrics() {}
    } lyrics;
    
    #endif // CAPTIONS
    

    class CoverArt: public Valid , public Action{
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
            if(s.w < 4 || s.h < 4)
                return;

            clog << "drawing image: " << s << " width: " << wm::WIDTH<< std::endl;
            std::string fpath = data.file_path.get_p();
            auto w = s.width() + (display_mode.vertical() ? 1 : 0);
            auto h = s.height() + 1;
            ascii_img::load_image_t *cover_ansi = audio::extra::getImg(fpath, w, h);
            clog <<  "cover_ansi: x: " <<  cover_ansi->x << " y: " <<  cover_ansi->y << std::endl;
            std::ostringstream out;
            for (size_t iy = 0; iy < h; iy++)
            {
                out << mv_str(s.x, s.y + iy);
                for (size_t ix = 0; ix < w; ix++)
                {
                    auto rgb = cover_ansi->get(ix + w * iy);

                    out << color_bg_str(clamp_1(rgb.r), clamp_1(rgb.g), clamp_1(rgb.b)) << ' ' << attr_reset;
                }
            }

            if (cover_ansi)
                delete cover_ansi;
            std::cout << out.str();
        };


        wm::Position drag_resize_start = {65535U, 65535U};
        bool enable_mid_drag_draw = true;
        void m_vert(wm::MOUSE_INPUT m){
            //start resize 
            if(m.pos.y == element.space.y && m.btn == wm::MOUSE_BTN::M_LEFT){
                drag_resize_start = m.pos;
            }//midway of resize
            else if(enable_mid_drag_draw && m.btn == wm::MOUSE_BTN::M_LEFT_HILIGHT && drag_resize_start.x != 65535U && drag_resize_start.y != 65535U){
                playlist_coverart_ratio = m.pos.y / static_cast<double>(wm::HEIGHT);
                elements_refresh = true;
                invalidate();
                p.invalidate();
            } // finalize resize
            else if(m.btn == wm::MOUSE_BTN::M_RELEASE && drag_resize_start.x != 65535U && drag_resize_start.y != 65535U){
                playlist_coverart_ratio = m.pos.y / static_cast<double>(wm::HEIGHT);
                clog << "resize  end: " << m.pos << ':' << playlist_coverart_ratio << std::endl;
                drag_resize_start = {65535U, 65535U};
                invalidate();
                p.invalidate();

            } // toggle between override image and album cover
            else if(element.wSpace().inside(m.pos) && m.btn == wm::MOUSE_BTN::M_LEFT){
                data.override = !data.override;
                data.img_valid = false;
            }

            clip_n(playlist_coverart_ratio, 0 ,1);
        }

        void m_hori(wm::MOUSE_INPUT m){
            if(m.pos.x == element.space.x && m.btn == wm::MOUSE_BTN::M_LEFT){
                drag_resize_start = m.pos;
            }//midway of resize
            else if(enable_mid_drag_draw && m.btn == wm::MOUSE_BTN::M_LEFT_HILIGHT && drag_resize_start.x != 65535U && drag_resize_start.y != 65535U){
                playlist_coverart_ratio = m.pos.x / static_cast<double>(wm::WIDTH);
                elements_refresh = true;

                invalidate();
                p.invalidate();
            } // finalize resize
            else if(m.btn == wm::MOUSE_BTN::M_RELEASE && drag_resize_start.x != 65535U && drag_resize_start.y != 65535U){
                playlist_coverart_ratio = m.pos.x / static_cast<double>(wm::WIDTH);
                drag_resize_start = {65535U, 65535U};

                invalidate();
                p.invalidate();

            } // toggle between override image and album cover
            else if(element.wSpace().inside(m.pos) && m.btn == wm::MOUSE_BTN::M_LEFT){
                data.override = !data.override;
                data.img_valid = false;
            }

            clip_n(playlist_coverart_ratio, 0 ,1);
        }

        void m_action(wm::MOUSE_INPUT m) override {
            if(display_mode.vertical()){
                m_vert(m);
            }
            else if(display_mode.horizontal()){
                m_hori(m);
            }
            
        }

        void invalidate() override{
            valid = false;
        }

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
    const char* IDS_NAMES[] = {
        "NO_ID",
        "PREV",
        "NEXT",
        "SHUFFLE",
        "PLAYPAUSE",
        "LOOP",
        "VOLUME",
        "TIME"
    };

    ENUM(GROUPS, short,
        UNGROUPED,
        MEDIA_CONTROLS,
        VOLUME,
        TIME
    );
    const char* GROUPS_NAMES[] = {
        "UNGROUPED",
        "MEDIA_CONTROLS",
        "VOLUME",
        "TIME"
    };

    namespace ControlButtons
    {

        class Controls :public ButtonArray, public Valid{};
        ButtonArray* active_button_array;

        struct Prev: public ButtonArrayElement, Action{
            //extra shit
            static StringLite ch;
            bool valid = false;
            bool action_inside = false;
            char ch_action_char = 'b';
            
            void ch_action(int ch) override{
                if(input_mode == InputMode::COMMON){
                    if(ch == ch_action_char)
                        load::prev();
                }
            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        load::prev();
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            };
            Prev(ButtonArrayElement b) : ButtonArrayElement(b){}
        } prev_bae = ButtonArrayElement{
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

        struct Next : public ButtonArrayElement, Action{
            static StringLite ch;
            bool valid = false;
            bool action_inside = false;
            char ch_action_char = 'n';
            void ch_action(int ch) override{
                if(input_mode == InputMode::COMMON){
                    if(ch == ch_action_char)
                        load::next();
                    
                }

            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        load::next();
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            };

            Next(ButtonArrayElement b): ButtonArrayElement(b) {};
        } next_bae = ButtonArrayElement{
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

        struct PlayPause: public ButtonArrayElement, Action
        {
            static StringLite playing_ch;
            static StringLite stopped_ch;
            enum State : unsigned char{
                INVALID,
                PLAYING,
                STOPPED,
            } state;
            bool action_inside = false;
            char ch_action_char = 'c';

            void ch_action(int c) override{
                if(input_mode == InputMode::COMMON){
                    if (c == ch_action_char)
                        control::playpause();
                }
            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        control::playpause(); //load::next();
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            }
            /* data */
            PlayPause(ButtonArrayElement b): ButtonArrayElement(b) {};
        } playpause_bae = ButtonArrayElement{
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

        struct Shuffle: public ButtonArrayElement, Action {
            static StringLite shuffle_ch;
            static StringLite sort_ch;
            enum State : unsigned char {
                INVALID,
                SORTED,
                SHUFFLED,
            } state = State::INVALID;
            bool action_inside = false;
            char ch_action_char = 's';

            void ch_action(int c) override{
                if(input_mode == InputMode::COMMON){
                    if (c == ch_action_char)
                        control::shuffle();
                }
            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        control::shuffle(); //load::next();
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            }
            Shuffle(ButtonArrayElement b): ButtonArrayElement(b) {};

        } shuffle_bae = ButtonArrayElement{
            [](bool inside, wm::MOUSE_INPUT m){
                use_attr(attr_reset);
                auto& usech = p.sorted() ? shuffle_bae.sort_ch : shuffle_bae.shuffle_ch;
                if(inside){
                    use_attr(color_color(colors.hover));
                    if(m.btn == wm::MOUSE_BTN::M_LEFT && p.size()){
                        control::shuffle();
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

        struct Loop : ButtonArrayElement, Action {
            static StringLite track_ch;
            static StringLite playlist_ch;
            static StringLite off_ch;
            enum State : unsigned char {
                INVALID,
                OFF,
                PLAYLIST,
                TRACK,
            } state = INVALID;
            bool action_inside = false;
            char ch_action_char = 'l';

            void refresh_state(){
                switch (p.loopType)
                {
                case Playlist::LoopType::L_NONE:
                    state = State::OFF;
                    break;
                case Playlist::LoopType::L_PLAYLIST:
                    state = State::PLAYLIST;
                    break;
                case Playlist::LoopType::L_TRACK:
                    state = State::TRACK;
                    break;
                default:
                    state = State::INVALID;
                    break;
                }
            }

            void next_loop_type(){
                Playlist::LoopType& l = p.loopType;
                switch (l)
                {
                case Playlist::LoopType::L_NONE:
                    l = Playlist::LoopType::L_PLAYLIST;
                    break;
                case Playlist::LoopType::L_PLAYLIST:
                    l = Playlist::LoopType::L_TRACK;
                    break;
                case Playlist::LoopType::L_TRACK:
                    l = Playlist::LoopType::L_NONE;
                    break;
                default:
                    break;
                }
                refresh_state();
                invalidate();
            }

            void ch_action(int c) override{
                if(input_mode == InputMode::COMMON){
                    if (c == ch_action_char)
                        next_loop_type();
                }
            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        next_loop_type(); //load::next();
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            }
            Loop(ButtonArrayElement b): ButtonArrayElement(b) {};
        } loop_bae = ButtonArrayElement{
        [](bool inside, wm::MOUSE_INPUT m)
            {
                //clog << "Refresh Looop" << std::endl;
                use_attr(attr_reset);
                StringLite &use_char = p.loopType == p.L_NONE ? loop_bae.off_ch : p.loopType == p.L_PLAYLIST? loop_bae.playlist_ch : loop_bae.track_ch;
                
                if (inside)
                {
                    use_attr(color_color(colors.hover));
                    if (m.btn == wm::MOUSE_BTN::M_LEFT)
                    {
                        loop_bae.next_loop_type();
                    }
                }
                else
                    use_attr(color_color(colors.normal));
                std::cout << use_char << " " << attr_reset;
                loop_bae.refresh_state();
                //loop_bae.state =  p.loopType == p.L_NONE ? Loop::OFF : p.loopType == p.L_PLAYLIST? Loop::PLAYLIST : Loop::TRACK;
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

        struct Volume : public ButtonArrayElement, Action{
            int vol = 0;
            bool valid = false;
            bool action_inside = false;
            char ch_action_char_up = '+';
            char ch_action_char_down = '-';

            void ch_action(int c) override{
                if(input_mode == InputMode::COMMON){
                    if(c == ch_action_char_up){
                        control::volume_rel(volume_shift);
                    }
                    else if(c == ch_action_char_down){
                        control::volume_rel(-volume_shift);
                    }
                }
            }

            //void k_action(wm::KEY k) override{
            //    if(k == wm::K_UP){
            //        control::volume_rel(volume_shift);
            //    }
            //    else if(k == wm::K_DOWN){
            //        control::volume_rel(-volume_shift);
            //    }
            //}
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        control::volume_set(volume_reset);
                    }else if(m.btn == wm::MOUSE_BTN::M_SCRL_UP){
                        control::volume_rel(volume_shift);
                    }else if(m.btn == wm::MOUSE_BTN::M_SCRL_DOWN){
                        control::volume_rel(-volume_shift);
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            }
            Volume(ButtonArrayElement b): ButtonArrayElement(b){}
        } volume_bae = ButtonArrayElement{
    [](bool inside, wm::MOUSE_INPUT m)
        {
            use_attr(attr_reset);
            // functionality
            if (inside)
            {
                use_attr(color_color(colors.hover));
            }
            else
                use_attr(color_color(colors.normal));

            // drawing
            volume_bae.vol = std::abs( (int) std::round(audio::volume::get()) );
            //clog << "Volume: " << audio::volume::get() << std::endl;
            volume_bae.vol = volume_bae.vol > 999 ? 999 : volume_bae.vol;
            std::cout << std::setfill('0') << std::setw(3) << std::right << volume_bae.vol << '%' << ' ' << attr_reset;
            volume_bae.valid = true;
        },
        5,
        GROUPS::VOLUME,
        IDS::VOLUME,
        []{
                //clog << "vbae.valid = " << (volume_bae.valid ? "True":"False") << std::endl;
                //clog << "vbae.vol = " << volume_bae.vol << '/' << std::abs( (int) std::round(audio::volume::get())) << " : " << ( std::abs( (int) std::round(audio::volume::get()) ) == volume_bae.vol ? "True":"False") << std::endl;
                return volume_bae.valid && std::abs( (int) std::round(audio::volume::get()) ) == volume_bae.vol;
            },
        []{volume_bae.valid = false;},

        };

        #define leading_zero(n) ((n < 10) ? "0" : "") << n
        struct Time : public Action, public ButtonArrayElement{
            long long current_time = 0;
            bool valid = false;
            bool action_inside = false;
            char action_ch_seek_fwd = 'x';
            char action_ch_seek_bwd = 'z';

            void ch_action(int c) override{
                if(input_mode == InputMode::COMMON){
                    if(c == action_ch_seek_fwd){
                        control::seek_rel(std::chrono::seconds(5));
                    }else if (c == action_ch_seek_bwd) {
                        control::seek_rel(std::chrono::seconds(-5));
                    }
                }
            }
            void k_action(wm::KEY k) override{
                if(input_mode != InputMode::COMMON)
                    return;

                if(k == wm::KEY::K_LEFT){
                    control::seek_rel(std::chrono::seconds(-5));
                    this->invalidate();

                }
                else if(k == wm::KEY::K_RIGHT){
                    control::seek_rel(std::chrono::seconds(5));

                    this->invalidate();
                }
            }
            void m_action(wm::MOUSE_INPUT m) override{
                auto p = active_button_array->getPosId(this->id);
                if(p == wm::Position{0,0}){
                    return;
                }
                wm::Space s = {p.x,p.y,this->alloc,0 /*maby 1 idk*/};
                if(m.pos.y == p.y && m.pos.x >= p.x && m.pos.x < p.x+this->alloc){
                    if(m.btn == wm::MOUSE_BTN::M_LEFT){
                        //alt display type?
                    }
                    if(!action_inside){
                        invalidate();
                    }
                    action_inside = true;
                }
                else{
                    if(action_inside)
                        invalidate();

                    action_inside = false;
                }
            }
            Time(ButtonArrayElement b) : ButtonArrayElement(b) {};
        } time_bae = ButtonArrayElement{
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
                time_bae.valid = true;
                time_bae.current_time = ps;
                #if defined (MPRIS)
                s.set_position(audio::position::get<MPRIS_TIME_RATIO>().count());
                #endif

            },
            11,
            GROUPS::TIME,
            IDS::TIME,
            []{
                //clog << "Time: " << time_bae.valid << ':' << (time_bae.current_time == audio::position::get<std::ratio<1, 1>>().count()) << std::endl;
                return time_bae.valid && time_bae.current_time == audio::position::get<std::ratio<1, 1>>().count();
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
                    //clog << "Controls:IsValid: "<< e.is_valid() << " Group: " << GROUPS_NAMES[e.group] << " ID: " << IDS_NAMES[e.id] << std::endl;
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

        Controls() {
            ControlButtons::active_button_array = this;
        };
        Controls(const std::vector<ButtonArrayElement>& v): ButtonArray(v) {
            ControlButtons::active_button_array = this;
        };

        ~Controls() {
            ControlButtons::active_button_array = nullptr;
        };

    } ctrl(controlButtons);

    struct Title : public Valid
    {
        wm::Element element;


        DisplayFileMode title = DisplayFileMode::METADATA_TITLE;
        DisplayFileMode window = DisplayFileMode::METADATA_TITLE;
        DisplayFileMode meda = DisplayFileMode::METADATA_TITLE;

        wm::SPLICE_TYPE clip_type = wm::SPLICE_TYPE::BEGIN_DOTS;
        wm::PAD_TYPE pad_type = wm::PAD_TYPE::PAD_CENTER;

        bool valid = false;

        bool is_valid() override{
            return valid;
        }

        bool need_to_get_metadata(){
            return title == METADATA_TITLE || window == METADATA_TITLE || meda == METADATA_TITLE;
        }

        void setTitle(std::string str, std::optional<audio::extra::AudioMetadata>& o_md){
            str = DisplayFileParse(str, title, o_md);
            set_title(str.c_str());
        }

        void setWindow(std::string str, std::optional<audio::extra::AudioMetadata>& o_md){
            auto s = element.aSpace();
            clog << "   setWindow:str: " << str << std::endl;
            str= DisplayFileParse(str, window, o_md);
            clog << "   setWindow:str: " << str << std::endl;

            //do some doing :3
            wm::clip(str, s.width(),    clip_type);
            wm::pad(str, s.width(),     pad_type);
            mv(s.x, s.y);
            std::cout << str;
        }

        void setMetadata( std::string str,std::optional<audio::extra::AudioMetadata>& o_md){
                DisplayFileParse(str, meda, o_md);
                
                #if defined (MPRIS)
                metadata.set(mpris::Field::Title, str);
                #endif         
            }

        void refresh() override{
            clog << "Refresh Title" << std::endl;
            valid = true;
            if(p.empty())
                return;
            //set window title
            //maby set metadata also but well do it later or something :3
            std::optional<audio::extra::AudioMetadata> md = std::nullopt;
            if(need_to_get_metadata()){
                auto o = p.opt_curr();
                if(o.has_value())
                    md = audio::extra::getMetadata(*o);
            }
            std::string str = p.opt_curr().value_or("");
            clog << "\t\tTitle:str: " << str << std::endl;
            setTitle(str, md);
            setWindow(str, md);
            setMetadata(str, md);

        }
    } title;
    


    
    
    

    

    namespace refresh
    {

        inline void elements_horizontal(wm::Space bounds){
            clog << "RESIZE HORIZONTAL:" << std::endl;
            clog << "playlist_coverart_ratio: " << playlist_coverart_ratio << std::endl;
            p.element.space = {
                bounds.start().x,
                bounds.start().y,
                static_cast<unsigned short>(bounds.width()*playlist_coverart_ratio),
                static_cast<unsigned short>(bounds.height())
            };
            p.element.pad = {1, 0, 0, 1};

            cover_art.element.space = {
                p.element.space.w,
                bounds.start().y,
                static_cast<unsigned short>(bounds.width()-p.element.space.w),
                static_cast<unsigned short>(bounds.height())
            };
            cover_art.element.pad = {1, 1, 1, 0};
        }



        inline void elements_vertical(wm::Space bounds){
            p.element.space = {
                bounds.start().x,
                bounds.start().y,
                static_cast<unsigned short>(bounds.width()),
                static_cast<unsigned short>(bounds.height()*playlist_coverart_ratio)
            };
            p.element.pad = {1, 0, 0, 0};

            cover_art.element.space = {
                p.element.space.x,
                static_cast<unsigned short>(p.element.space.end().y),
                static_cast<unsigned short>(bounds.width()),
                static_cast<unsigned short>(bounds.height()-p.element.space.h)
            };
            clog << "bounds.height(): " << bounds.height() << std::endl;
            cover_art.element.pad = {1, 1, 0, 0};
            clog << "Playlist[" << p.element << "] CoverArt[" << cover_art.element << "]" << std::endl;
        }

        #if defined( MPRIS)
        inline void metadata_r(){
            if(metadata.is_valid())
                return;
            clog << "Metadata Refresh" << std::endl;
            metadata.refresh();
        }
        #endif

        //every resize_event
        inline void elements(){
            elements_refresh  = false;
            clog << "ELEMENT RESIZE" << std::endl;

            title.element.space = wm::Space(0,0, wm::WIDTH, 1);
            #if defined(CAPTIONS)
            lyrics.element.space = wm::Space(0, title.element.space.end().y, wm::WIDTH, 1);
            #endif // CAPTIONS



            ctrl.pos = {static_cast<unsigned short>(wm::WIDTH - ctrl.width()), static_cast<unsigned short>(wm::HEIGHT)};
            progres_bar.element.space = wm::Space(0, wm::HEIGHT, ctrl.pos.x - 1 , 1);

            wm::Space bounds;
            bounds.x = 0;
            bounds.y = 
            #if defined(CAPTIONS)
            lyrics.element.space.end().y;
            #else
            title.element.space.end().y;
            #endif // CAPTIONS
            bounds.w = wm::WIDTH;
            bounds.h = (wm::HEIGHT - bounds.y) - progres_bar.element.space.h;

            
            if(display_mode.horizontal()){
                elements_horizontal(bounds);
            }
            else if(display_mode.vertical()){
                elements_vertical(bounds);
            }
        };

        //all the classes that have the refresh thing
        inline void playlist(){
            if(p.is_valid())
                return;
            clog << "Playlist Refresh" << std::endl;
            p.refresh();
            
        };
        inline void progressbar(){
            if(progres_bar.is_valid())
                return;
            //clog << "ProgressBar Refresh" << std::endl;
            progres_bar.refresh();
            
        };
        inline void coverart(){
            if(cover_art.is_valid())
                return;

            clog << "CoverArt Refresh" << std::endl;
            cover_art.refresh();
            
        };
        inline void config(){
            if(cfg.is_valid())
                return;
            clog << "Config Refresh" << std::endl;
            cfg.refresh();
            
        };
        //todo
        inline void controls(){
            if(ctrl.is_valid())
                return;
            
            //clog << "Controls Refresh" << std::endl;
            ctrl.draw({mpos, wm::MOUSE_BTN::M_NONE, true});
        };
        inline void r_title(){
            if(p.empty() || title.is_valid())
                return;
            title.refresh();
            clog << "Title Refresh" << std::endl;
        };

        inline void all(){
            if(wm::resize_event){
                wm::resize_event = false;

                //elements();

                clog << (display_mode.horizontal() ? "HORIZONTAL " : "VERTICAL ") << wm::WIDTH << '/' << wm::HEIGHT << std::endl; 
                clog << "RESIZE EVENT" << std::endl;
                elements_refresh = true;
                p.valid = false;
                progres_bar.valid = false;
                cover_art.valid = false;
                title.valid = false;
                ctrl.invalidate_all();
                #if defined(CAPTIONS)
                
                lyrics.invalidate();
                
                #endif // CAPTIONS
                
                //for(auto& e : ctrl){
                //    if(e.invalidate)
                //        e.invalidate();
                //}
                clear_scr();
            }
            if(elements_refresh)
                elements();
            //clog << p.element << std::endl << cover_art.element << std::endl;
            playlist();
            progressbar();
            controls();
            r_title();
            //metadata_r(); //idk why 2 but there are 2 now :3
            coverart();
            #if defined(CAPTIONS)
            
            if(!lyrics.is_valid())
                lyrics.refresh();
            #endif // CAPTIONS
            
            #if defined (MPRIS)
            metadata_r(); //idk why 2 but there are 2 now :3
            #endif
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
                //set track
            #if defined(MPRIS)
            metadata.set(mpris::Field::TrackId, '/'+std::to_string(p.current_index));
            metadata.set(mpris::Field::TrackNumber, static_cast<int>(p.current_index));
            //sdbus::Variant l = ;
            metadata.set(mpris::Field::Length, static_cast<int>(audio::duration::get<MPRIS_TIME_RATIO>().count()));
            #endif
            
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
            ctrl.invalidateId(IDS::PLAYPAUSE);

            #if defined(MPRIS)
            s.set_playback_status(audio::playing()? mpris::PlaybackStatus::Playing : mpris::PlaybackStatus::Paused );
            #endif // MPRIS
        }
        inline void play(){
            if(p.empty())
                return;

            audio::control::play();
            ctrl.invalidateId(IDS::PLAYPAUSE);

            #if defined(MPRIS)
            s.set_playback_status(mpris::PlaybackStatus::Playing);
            #endif // MPRIS
            
        }
        inline void pause(){
            if(p.empty())
                return;

            audio::control::pause();
            ctrl.invalidateId(IDS::PLAYPAUSE);

            #if defined(MPRIS)
            s.set_playback_status(mpris::PlaybackStatus::Paused);
            #endif // MPRIS
        };
        inline void shuffle(){
            if(p.empty())
                return;
            if(p.sorted()){
                if(!p.seed)
                    p.new_seed();
                p.shuffle();
                ctrl.invalidateId(IDS::SHUFFLE);
            }else{
                p.sort();
                ctrl.invalidateId(IDS::SHUFFLE);
            }
            p.valid = false;
        }

        inline void seek_abs(double d){
            audio::seek::abs(d);
        }
        template<typename T, typename Rat>
        inline void seek_rel(std::chrono::duration<T, Rat> d){
            audio::seek::rel(d);
        }

        inline void volume_set(double d){
            audio::volume::set(d);
            //ctrl.invalidateId(IDS::VOLUME);
            #if defined(MPRIS)
            s.set_volume(audio::volume::get()/100);
            #endif // MPRIS
        }
        inline void volume_rel(double d){
            audio::volume::shift(d);
            #if defined(MPRIS)
            s.set_volume(audio::volume::get()/100);
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
                    Config::path = optarg;
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
            audio::seek::rel(std::chrono::duration<long, MPRIS_TIME_RATIO>(p));
            s.set_position(audio::position::get<MPRIS_TIME_RATIO>().count());
        });
        s.on_set_position([&] (int64_t p) {
            clog << "s.on_set_position:p:" << p <<std::endl;
            audio::seek::abs(std::chrono::duration<long, MPRIS_TIME_RATIO>(p));
            s.set_position(audio::position::get<MPRIS_TIME_RATIO>().count());
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
            control::shuffle();
        });
        s.on_volume_changed([&] (double v) { //because why not :3
            double vol = v*100;
            //clog << "ovc: " << v << "vol: " << audio::volume::get() << std::endl;
            control::volume_set( vol > 100 ? 100 : vol);
            
        });
        
        s.set_volume(audio::volume::get()/100);

        s.start_loop_async();
        clog << "Mpris Initialized" << std::endl;
    }
    #endif // MPRIS

    std::chrono::milliseconds d(static_cast<long>((1./24.)*1000));

    inline void refresh_rate(size_t n){
        d = std::chrono::milliseconds(static_cast<long>((1./n)*1000));
    }
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
                p.shuffle();
            
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
    std::vector<Action*> actions({&p, &progres_bar, &cover_art, &ControlButtons::time_bae, &ControlButtons::prev_bae, &ControlButtons::next_bae, &ControlButtons::playpause_bae,  &ControlButtons::shuffle_bae, &ControlButtons::loop_bae, &ControlButtons::volume_bae});    

    inline void exec_action_ch(Action* a, int c){
        a->ch_action(c);
    }
    inline void exec_action_k(Action* a, wm::KEY c){
        a->k_action(c);
    }
    inline void exec_action_m(Action* a, wm::MOUSE_INPUT c){
        a->m_action(c);
    }

    inline void action(int i){ //parallell but synchronus
        std::vector<std::thread> thr;
        for (auto& a : actions)
            thr.emplace_back(exec_action_ch, a, i);
        for (auto& t : thr)
            t.join();
    }
    inline void action(wm::KEY i){
        std::vector<std::thread> thr;
        for (auto& a : actions)
            thr.emplace_back(exec_action_k, a, i);
        for (auto& t : thr)
            t.join();
    }
    inline void action(wm::MOUSE_INPUT i){
        std::vector<std::thread> thr;
        for (auto& a : actions)
            thr.emplace_back(exec_action_m, a, i);
        for (auto& t : thr)
            t.join();
    }



    inline void deinit(){
        #if defined(MPRIS)
        try {
        s.connection->leaveEventLoop();
        } catch (const sdbus::Error& er ) {
            std::cerr << er.getMessage() << '\n' << er.getName() << '\n' << er.what() << std::endl;
        }
        #endif // MPRIS

        if(audio::stopped())
            return;
        
        
    }

} // namespace temqo

