#include "lib/audio/playlist.hpp"
#include "lib/audio/extract_img.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/cfg/config.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/clip.hpp"
#include "lib/wm/core.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/getch.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/mouse.hpp"
#include "lib/wm/position.hpp"
#include "lib/wm/space.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <thread>

audio::Playlist pl;

enum INPUT_MODE : char{
    DEFAULT = 'D',
    SEARCH = 'S',
    COMMAND = 'C',
} current_mode = DEFAULT;


wm::Position mpos = {0,0};
std::string input;

float volume_shift = 5.f;
float volume_reset = 100.f;
bool exit_mainloop = false;
//COLORS
cfg::RGB warning_color_fg = {150, 60, 60};
cfg::RGB warning_color_bg = {0, 0, 0};
cfg::RGB hilight_color_fg = {20, 30, 20};
cfg::RGB hilight_color_bg = {200,200,200};
cfg::RGB hover_color_fg = {222,222,222};
cfg::RGB hover_color_bg = {60,60,60};
cfg::RGB progress_bar_color_played_fg = {255,0,0};
cfg::RGB progress_bar_color_played_bg = {0,0,0};
cfg::RGB progress_bar_color_remaining_fg = {200,200,200};
cfg::RGB progress_bar_color_remaining_bg = {0,0,0};
cfg::RGB progress_bar_color_cursor_fg = {255,255,255};
cfg::RGB progress_bar_color_cursor_bg = {0,0,0};
cfg::RGB border_color_fg = {193,119,1};
cfg::RGB border_color_bg = {0,0,0};


//char/strings
std::string progres_bar_char_first = "├";
std::string progres_bar_char_center = "─";
std::string progres_bar_char_last = "┤";
std::string progres_bar_char_cursor = "•";

//ease of use
int arrowkey_scroll_sensitivity = 1;
int mouse_scroll_sensitivity = 3;

wm::Element current_file;
wm::Element input_field;

wm::Element playlist;
wm::Element cover_art;

namespace UIelements
{
    int settings_alloc = 1;
    int time_played_alloc = 11;
    int volume_alloc = 4;
    int toggle_alloc = 1;
    bool settings_hover = false;
    bool time_played_hover = false;
    bool volume_hover = false;
    bool toggle_hover = false;
    wm::Element settings;
    wm::Element time_played;
    wm::Element volume;
    wm::Element toggle;
    /// @brief allo rest of the ui space
    bool playbar_hover = false;
    wm::Element playbar;
} // namespace UIelements


bool enable_cover = true;
bool cover_file_valid = false;
bool cover_art_valid = false;
std::string covert_img_path;

bool playlist_filename_only = true;
bool currently_playing_filename_only = true;
bool title_filename_only = true;


int playlist_display_offset = 0;
int playlist_cursor_offset = 0;
double playlist_width = 0.5;

size_t playlist_clamp(long i){
    while (i < 0)
    {
        i += pl.files.size();
    }
    if (i >= pl.files.size())
    {
        i = i % pl.files.size();
    }
    return i;
}
size_t playlist_st_index(){
    return playlist_clamp(playlist_display_offset);
}
size_t playlist_last_index(){
    return playlist_clamp(playlist_display_offset+playlist.wSpace().h);
}

void refresh_playlist(){
    //display the elements
    auto s = playlist.wSpace();
    playlist_cursor_offset = playlist_clamp(playlist_cursor_offset);
    for (size_t index = 0; index <= s.h; index++)
    {
        auto i = playlist_clamp(playlist_display_offset+index);
        mv(s.x, s.y+index);
        auto str = pl[i];
        if(playlist_filename_only) str = path::filename(str);
        str = std::to_string(i)+ ' ' +str;
        wm::clip(str, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str, s.width(), wm::PAD_TYPE::PAD_RIGHT);

        if(i == playlist_cursor_offset){
            use_attr(color_bg_rgb( hover_color_bg) << color_fg_rgb(hover_color_fg));
        }
        if(i == pl.current_index){
            use_attr(color_bg_rgb( hilight_color_bg) << color_fg_rgb(hilight_color_fg));
        }

        std::cout  << str << attr_reset;

        
    }
    //draw the borders
    {
        auto s= playlist.space;
        std::string b;
        for (size_t i = 0; i < s.width(); i++)
        {
            b+="─";
        }
        use_attr(color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg));
        mv(s.x, s.y);
        std::cout << b;
        mv(s.x, s.y+s.h);
        std::cout << b << attr_reset;
        
    }
    
}

void refresh_display_offset(){
    auto idx =playlist_clamp(playlist_cursor_offset);
    auto fst = playlist_st_index();
    auto lst = playlist_last_index();
    if(idx < fst){
        playlist_display_offset -=fst-idx;
    }
    if(idx > lst){
        playlist_display_offset -=lst-idx;
    }
}

void refresh_coverart_img(){
    auto res = audio::extra::extractAlbumCoverTo(pl.current(), TMP_OUT);
    covert_img_path = (res == 0) ? TMP_OUT : "";
    cover_file_valid = true;
}

void refresh_coverart(){
    cover_art_valid = true;
    use_attr(color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg));
    cover_art.space.box("─","─",nullptr, "│" , "┬", nullptr, "┴", nullptr);
    use_attr(attr_reset);
    if(!cover_file_valid){
        refresh_coverart_img();
    }
    if(covert_img_path.length() == 0){
        return;
    }
    auto s = cover_art.wSpace();
    auto cover_ansi = audio::extra::getImg(covert_img_path, s.width(), s.height() + 1);
    
    std::ostringstream out;
#define clamp_1(val) (val == 0) ? 1 : val
    for (size_t iy = 0; iy <= s.height(); iy++)
    {
        out << mv_str(s.x, s.y + iy);
        for (size_t ix = 0; ix < s.width(); ix++)
        {
            auto rgb = cover_ansi->get(ix + s.width() * iy);

            out << color_bg_str(clamp_1(rgb.r), clamp_1(rgb.g), clamp_1(rgb.b)) << ' ' << attr_reset;
        }
    }
    delete cover_ansi;
    std::cout << out.str();
    return;
}

void refresh_currently_playing(){
    std::string c = currently_playing_filename_only? path::filename(pl.current()): pl.current();
    auto s = current_file.aSpace();
    //current_file.space.fill("?");
    wm::clip(c, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
    wm::pad(c, s.width(), wm::PAD_TYPE::PAD_CENTER);
    mv(s.x, s.y);
    std::cout << c;
}
void refresh_settings(){
    auto pe = UIelements::settings.space;
    mv(pe.x, pe.y);
    auto inside = UIelements::settings_hover;

    if(inside){
        use_attr(color_bg_rgb( hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout <<  "⚙ ";
    if(inside){
        use_attr(attr_reset);
    }
}
#define leading_zero(n) ((n < 10) ? "0" : "") << n
void refresh_time_played(){
    auto s  =UIelements::time_played.space;
    mv(s.x,s.y);
    auto ps = audio::position::get<std::ratio<1,1>>().count();
    auto ts = audio::duration::get<std::ratio<1,1>>().count();

    auto pm = ps/60;
    auto tm = ts/60;

    auto psec = ps-pm*60; 
    auto tsec = ts-tm*60;

    bool inside = UIelements::time_played_hover;
    if(inside){
        use_attr(color_bg_rgb( hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout 
    << leading_zero(pm) << ':' << leading_zero(psec) 
    << '/' 
    << leading_zero(tm) << ':' << leading_zero(tsec);
    if(inside){
        use_attr(attr_reset);
    }
}
#undef leading_zero

void refresh_volume(){
    auto s = UIelements::volume.space;
    mv(s.x,s.y);
    int vol = (int) audio::volume::get();
    if(vol < 0){
        vol = -vol;
    }
    if(vol > 999){
        vol = 999;
    }
    bool inside = UIelements::volume_hover;
    if(inside){
        use_attr(color_bg_rgb( hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout << std::setfill('0') << std::setw(s.w-1) << vol << '%';
    if(inside){
        use_attr(attr_reset);
    }
    std::cout<< ' ';

}

void refresh_play_button(){
    auto s = UIelements::toggle.space;
    mv(s.x,s.y);
    bool inside = UIelements::toggle_hover;
    if(inside){
        use_attr(color_bg_rgb( hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout << std::setw(s.w-1) << (audio::playing() ? "⏵" : "⏸");
    if(inside){
        use_attr(attr_reset);
    }
    std::cout<< ' ';
}

void refresh_UIelements(){
    refresh_settings();
    refresh_time_played();
    refresh_volume();
    refresh_play_button();
}
void refresh_playbar(){
    if(current_mode == INPUT_MODE::DEFAULT){
        auto d = audio::position::get_d();
        auto s = UIelements::playbar.space;
        auto idx = static_cast<int>(s.w*d);
        mv(s.x, s.y);
        use_attr(color_bg_rgb(progress_bar_color_played_bg) << color_fg_rgb(progress_bar_color_played_fg));
        for (size_t i = 0; i < s.width(); i++)
        {
            if(i == idx){
                use_attr(attr_reset << color_bg_rgb(progress_bar_color_cursor_bg) << color_fg_rgb(progress_bar_color_cursor_fg));
                std::cout << progres_bar_char_cursor;
                use_attr(attr_reset << color_bg_rgb(progress_bar_color_remaining_bg) << color_fg_rgb(progress_bar_color_remaining_fg));
            }
            else if(i == 0){
                std::cout << progres_bar_char_first;
            }
            else if(i == s.width()-1){
                std::cout << progres_bar_char_last;
            }
            else{
                std::cout << progres_bar_char_center;
            }
        }
        use_attr(attr_reset);
    }
    else if(current_mode == INPUT_MODE::COMMAND){
        auto s = UIelements::playbar.space;
        auto str = ':'+input;
        wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str,s.w,wm::PAD_TYPE::PAD_RIGHT);
        mv(s.x, s.y);
        std::cout << str;
    }
    else if(current_mode == INPUT_MODE::SEARCH){
        auto s = UIelements::playbar.space;
        auto str = '/'+input;
        wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str,s.w,wm::PAD_TYPE::PAD_RIGHT);
        mv(s.x, s.y);
        std::cout << str;
    }

}
void refresh_element_sizes(){
    current_file.space = wm::Space(0,0, wm::WIDTH, 0);
    input_field.space =  wm::Space(0, wm::HEIGHT, wm::WIDTH, 0);
    
    playlist.space = wm::Space(0,1, wm::WIDTH*playlist_width, wm::HEIGHT-2);
    playlist.pad = {1,1,0,0};
    cover_art.space = wm::Space(playlist.space.w,1, wm::WIDTH-playlist.space.w+1, wm::HEIGHT-2);
    cover_art.pad = {1,1,1,0};
    //implement album cover


    //order is important
    UIelements::settings.space      =   wm::Space(wm::WIDTH - UIelements::settings_alloc,                           wm::HEIGHT, UIelements::settings_alloc,     0);
    UIelements::time_played.space   =   wm::Space(UIelements::settings.space.x -1 -UIelements::time_played_alloc,   wm::HEIGHT, UIelements::time_played_alloc,  0);
    UIelements::volume.space        =   wm::Space(UIelements::time_played.space.x-1-UIelements::volume_alloc,       wm::HEIGHT, UIelements::volume_alloc,       0);
    UIelements::toggle.space        =   wm::Space(UIelements::volume.space.x-1-UIelements::toggle_alloc,            wm::HEIGHT, UIelements::toggle_alloc,       0);
    UIelements::playbar.space       =   wm::Space(0,                                                                wm::HEIGHT, UIelements::toggle.space.x-2,   0);
}
//veri expensiv
void refresh_all(){
    clear_all();
    use_attr(cursor_invisible);
    refresh_element_sizes();
    refresh_currently_playing();
    refresh_playlist();
    refresh_UIelements();
    refresh_playbar();
    try{
        refresh_coverart();
    }catch(...){}
}

void load_file(std::string filepath){
    
    cover_file_valid = false;
    cover_art_valid = false;
    std::thread thr(audio::load,filepath);

    set_title(
        (title_filename_only ? path::filename(filepath) : filepath).c_str()
    );
    playlist_cursor_offset = pl.current_index;
    refresh_display_offset();

    //audio::load(filepath);
    refresh_currently_playing();
    refresh_playlist();

    thr.join();
    audio::control::play();
    refresh_playbar();
    refresh_UIelements();
}


void deinit() {
    audio::control::pause();
    wm::deinit();
}
void csig(int sig){
    audio::control::pause();
    wm::deinit();
    exit(0);
}


void handle_resize(){
    wm::resize_event = false;
    cover_art_valid = false;
    refresh_all();
}



void handle_mouse(wm::MOUSE_INPUT m){
    mpos = m.pos;
    

    if(UIelements::settings.space.inside(m.pos) != UIelements::settings_hover){
        UIelements::settings_hover = UIelements::settings.space.inside(m.pos);
        refresh_settings();
    }
    if(UIelements::time_played.space.inside(m.pos) != UIelements::time_played_hover){
        UIelements::time_played_hover = UIelements::time_played.space.inside(m.pos);
        refresh_time_played();
    }
    if(UIelements::volume.space.inside(m.pos) != UIelements::volume_hover){
        UIelements::volume_hover = UIelements::volume.space.inside(m.pos);
        refresh_volume();
    }
    if(UIelements::volume.space.inside(m.pos)){
        if(m.btn == wm::MOUSE_BTN::M_SCRL_UP){
            audio::volume::shift(volume_shift);
        }
        else if(m.btn == wm::MOUSE_BTN::M_SCRL_DOWN){
            audio::volume::shift(-volume_shift);
        }
        else if(m.btn == wm::MOUSE_BTN::M_LEFT){
            audio::volume::set(volume_reset);
        }
        refresh_volume();
    }
    if(UIelements::toggle.space.inside(m.pos) != UIelements::toggle_hover){
        UIelements::toggle_hover = UIelements::toggle.space.inside(m.pos);
        refresh_play_button();
    }
    if(UIelements::toggle_hover && m.btn == wm::MOUSE_BTN::M_LEFT){
        audio::control::toggle();
        refresh_play_button();
    }
    if(UIelements::playbar.space.inside(m.pos) && m.btn == wm::MOUSE_BTN::M_LEFT){
        auto delta = static_cast<double>(m.pos.x)/static_cast<double>(UIelements::playbar.space.width());
        audio::seek::abs(delta);
        refresh_time_played();
        refresh_playbar();
    }
    
    if(playlist.wSpace().inside(m.pos)){
        if(m.btn == wm::MOUSE_BTN::M_SCRL_UP){
            playlist_cursor_offset-=mouse_scroll_sensitivity;
            refresh_display_offset();
        }
        else if(m.btn == wm::MOUSE_BTN::M_SCRL_DOWN){
            playlist_cursor_offset+=mouse_scroll_sensitivity;
            refresh_display_offset();
        }
        else{
            auto s = playlist.wSpace();
            auto i = m.pos.y - s.y;
            playlist_cursor_offset = playlist_clamp(playlist_display_offset+i);
            refresh_display_offset();
            if(m.btn == wm::MOUSE_BTN::M_LEFT){
                pl.current_index = playlist_cursor_offset;
                load_file(pl.current());
            }
        }

        
        refresh_playlist();
    }

}

void handle_arrow_key(wm::KEY k){
    switch (k) {
        case wm::K_UP:
            playlist_cursor_offset-=arrowkey_scroll_sensitivity;
            refresh_display_offset();
            refresh_playlist();
            break;
        case wm::K_DOWN:
            playlist_cursor_offset+=arrowkey_scroll_sensitivity;
            refresh_display_offset();
            refresh_playlist();
            break;
        case wm::K_RIGHT:
            audio::seek::rel(std::chrono::seconds(5));
            break;
        case wm::K_LEFT:
            audio::seek::rel(std::chrono::seconds(-5));
            break;
        default:
            return;
    }
}

void update_input(){

}

namespace hcr
{
    std::regex add_to_playlist("^p?l?add .*$");
    std::regex use_playlist("^p?l?use .*$");
    std::regex save_playlist("^p?l?save *$");
    std::regex saveto_playlist("^p?l?saveto .*$");
    std::regex use_config("^useco?n?fi?g .*$");
} // namespace hcr


void handle_command(){
    try{
        if(input.length() == 0){
            return;
        }
        if(std::regex_match(input, hcr::save_playlist)){
            pl.save();
        }
        else if(std::regex_match(input, hcr::saveto_playlist)){
            pl.save(input.substr(input.find_first_of(' ')+1));
        }
        else if(std::regex_match(input, hcr::add_to_playlist)){
            pl.add(input.substr(input.find_first_of(' ')+1));
        }
        else if(std::regex_match(input, hcr::use_playlist)){
            pl.save();
            pl.use(input.substr(input.find_first_of(' ')+1));
        }
        else if(std::regex_match(input, hcr::use_config)){
            cfg::parse(input.substr(input.find_first_of(' ')+1));
        }
    }
    catch(...){
        return;
    }
}
void handle_search(){
    size_t index = std::string::npos;
    std::string out = pl.find_insensitive(input, &index);

    if(out.length() == 0 || index == std::string::npos){
        return;
    }
    playlist_cursor_offset = index;
    refresh_display_offset();
    refresh_playlist();
}
//add the next found to the search shit
void handle_char(int ch){
    auto c = (char)ch;
    if (current_mode == DEFAULT) //yea the mode shit
    {
        switch (c) {
            case 'q':
                exit_mainloop = true;
                break;
            case 'n':
                load_file(pl.next());
                break;
            case 'b':
                load_file(pl.prev());
                break;
            case 'c':
                audio::control::toggle();
                break;
            case '+':
                audio::volume::shift(volume_shift);
                break;
            case '-':
                audio::volume::shift(-volume_shift);
                break;
            case '\n':
                pl.current_index = playlist_clamp(playlist_cursor_offset);
                load_file(pl.current());
                break;
                //switch to command mode
            
            case '/':
                input = "";
                current_mode = INPUT_MODE::SEARCH;
                break;
            case 'i':
            case ':':
                input = "";
                current_mode = INPUT_MODE::COMMAND;
                break;
            default:
                break;
        }
    }
    else if(current_mode == COMMAND){
        switch (c) {
            case '\n':
                handle_command();
                input = "";
                current_mode = DEFAULT;
                break;
            case '\b':
            case static_cast<char>(127):
                if(input.length() > 0){
                    input.pop_back();
                }
                break;
            default:
                input+=c;
                break;
        }
    }
    else if (current_mode == SEARCH){
        switch (c) {
            case '\n':
                pl.current_index = playlist_clamp(playlist_cursor_offset);
                load_file(pl.current());
                input = "";
                current_mode = DEFAULT;
                break;
            case '\b':
            case static_cast<char>(127):
                if(input.length() > 0){
                    input.pop_back();
                }
                handle_search();
                break;
            default:
                input+=c;
                handle_search();
                break;
        }
    }
    refresh_playbar();

}

void handle_input(int ch){
    //std::cout<< std::hex <<  std::setw(4*2) << ch;
    if (ch == 0x1b1b)
    {
        input = "";
        current_mode = INPUT_MODE::DEFAULT;
        return;
    }
    if(is_mouse(ch)){
        handle_mouse(wm::parse_mouse(ch));
        return;
    }
    if(auto k = wm::is_key(ch)){
        handle_arrow_key(k);
        return;
    }
    if(ch > 0 && ch < 255){
        handle_char(ch);
    }

    
    
}

std::chrono::duration sleep_for = std::chrono::milliseconds(100);
bool refrehs_thread_alive = true;
bool in_getch = false;
void refrehs_thread(){
    while(refrehs_thread_alive){
        if(!in_getch){
            std::this_thread::sleep_for(sleep_for);
            continue;
        }
        if(audio::stopped()){
            load_file(pl.next());
        }
        if(wm::resize_event){
            handle_resize();
        }
        if(!cover_art_valid){
            try{
                refresh_coverart();
            }catch(...){}
        }
        refresh_time_played();
        refresh_playbar();
        std::this_thread::sleep_for(sleep_for);
    }
    
}

void configuraton(){
    cfg::add_config_inline("HilightColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        hilight_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        hilight_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });
    cfg::add_config_inline("WarnColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        warning_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        warning_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });
    cfg::add_config_inline("HoverColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        hover_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        hover_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });
    cfg::add_config_inline("BorderColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        border_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        border_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); 
    });
    ///Playlist
    cfg::add_config_inline("PlaylistFilenameOnly", [](std::string line){
        auto str = cfg::parse_inline(line);
        playlist_filename_only = cfg::parse_bool(str);
    });
    cfg::add_config_inline("PlayingFilenameOnly", [](std::string line){
        auto str = cfg::parse_inline(line);
        currently_playing_filename_only = cfg::parse_bool(str);
    });
    cfg::add_config_inline("TitleFilenameOnly", [](std::string line){
        auto str = cfg::parse_inline(line);
        title_filename_only = cfg::parse_bool(str);
    });

    ///Progress Bar
    cfg::add_config_inline("ProgresBarPlayedColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_played_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_played_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });
    cfg::add_config_inline("ProgresBarRemainingColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_remaining_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_remaining_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });
    cfg::add_config_inline("ProgresBarCursorColor", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_cursor_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_cursor_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1));
    });

    //i hope this works
    cfg::add_config_inline("ProgresBarChar", [](std::string line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        auto f      = cfg::get_bracket_contents(str, &idx, 0    );
        auto c      = cfg::get_bracket_contents(str, &idx, idx+1);
        auto l      = cfg::get_bracket_contents(str, &idx, idx+1);
        auto curs   = cfg::get_bracket_contents(str, &idx, idx+1);
        progres_bar_char_first = (f.length()>0) ? f : progres_bar_char_first;
        progres_bar_char_center = (c.length()>0) ? c : progres_bar_char_center;
        progres_bar_char_last = (l.length()>0) ? l : progres_bar_char_last;
        progres_bar_char_cursor = (curs.length()>0) ? curs : progres_bar_char_cursor;
    });

    ///MISC
    cfg::add_config_inline("CoverArt", [](std::string line){
        auto str = cfg::parse_inline(line);
        enable_cover = cfg::parse_bool(str);
    });
    cfg::add_config_inline("Volume", [](std::string line){
        auto str = cfg::parse_inline(line);
        auto vol = std::stoi(str);
        audio::volume::set(vol);
    });
    
}
void init(int argc, char const *argv[]){
    //signals
    atexit(deinit);
    signal(SIGINT, csig);

    //window thingies
    wm::init();
    refresh_element_sizes();
    use_attr(cursor_invisible);
    //config
    configuraton();
    cfg::parse("temqo.cfg");
    //playlist
    const char *filename = argc > 2 ? argv[2] : "playlist.pls";
    auto seek_to = pl.use(filename);
    //"⤭" shall be used to add a shuffle feature
    //audio server
    load_file(pl.current());
    playlist_cursor_offset = pl.current_index;
    playlist_display_offset = pl.current_index;
    audio::seek::abs(std::chrono::seconds(seek_to));
}

int main(int argc, char const *argv[])
{
    init(argc, argv);
    refresh_all();
    std::thread thr(refrehs_thread);
    use_attr(cursor_invisible);
    while (!exit_mainloop) {
        if(wm::resize_event){
            handle_resize();
        }
        in_getch= true;
        int ch = wm::getch();
        in_getch= false;
        handle_input(ch);

    }
    refrehs_thread_alive = false;
    try{
        pl.save();
    }
    catch(...){}
    deinit();
    thr.join();
    return 0;
}

