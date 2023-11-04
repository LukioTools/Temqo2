#include "lib/audio/playlist.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/cfg/config.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/clip.hpp"
#include "lib/wm/core.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/mouse.hpp"
#include "lib/wm/position.hpp"
#include "lib/wm/space.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

audio::Playlist pl;

wm::Position mpos = {0,0};


//COLORS
cfg::RGB warning_color_fg = {150, 60, 60};
cfg::RGB warning_color_bg = {0, 0, 0};
cfg::RGB hilight_color_fg = {20, 30, 20};
cfg::RGB hilight_color_bg = {200,200,200};
cfg::RGB hover_color_bg = {60,60,60};
cfg::RGB hover_color_fg = {222,222,222};
cfg::RGB progress_bar_color_played_bg = {0,0,0};
cfg::RGB progress_bar_color_played_fg = {255,0,0};
cfg::RGB progress_bar_color_remaining_bg = {0,0,0};
cfg::RGB progress_bar_color_remaining_fg = {200,200,200};
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


bool cover_valid = false;
std::string covert_img_path;

bool playlist_filename_only = true;
bool currently_playing_filename_only = true;
bool title_filename_only = true;

int playlist_display_offset = 0;
int playlist_cursor_offset = 0;

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
        mv(s.x, s.y);
        std::cout << b;
        mv(s.x, s.y+s.h);
        std::cout << b;
    }
    
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
    //UIelements::time_played.space.fill("T");
    //UIelements::volume.space.fill("V");
    //UIelements::toggle.space.fill("C");
}
void refresh_playbar(){
    //UIelements::playbar.space.fill("─");
}
void refresh_element_sizes(){
    current_file.space = wm::Space(0,0, wm::WIDTH, 0);
    input_field.space =  wm::Space(0, wm::HEIGHT, wm::WIDTH, 0);
    
    playlist.space = wm::Space(0,1, wm::WIDTH, wm::HEIGHT-2);
    playlist.pad = {1,1,0,0};
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
    refresh_element_sizes();
    refresh_currently_playing();
    refresh_playlist();
    refresh_UIelements();
    refresh_playbar();
}

void load_file(std::string filepath){
    cover_valid = false;
    audio::load(filepath);
    set_title(
        (title_filename_only ? path::filename(filepath) : filepath).c_str()
    );
    audio::control::play();
    refresh_currently_playing();
    refresh_playlist();
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

void init(int argc, char const *argv[]){
    //signals
    atexit(deinit);
    signal(SIGINT, csig);

    //window thingies
    wm::init();
    refresh_element_sizes();
    use_attr(cursor_invisible);

    //playlist
    const char *filename = argc > 2 ? argv[2] : "playlist.pls";
    auto seek_to = pl.use(filename);
    //audio server
    load_file(pl.current());
    //playlist_cursor_offset = pl.current_index;
    //playlist_display_offset = pl.current_index;
    audio::seek::abs(std::chrono::seconds(seek_to));
}
void handle_resize(){
    wm::resize_event = false;
    refresh_all();
}
void handle_mouse(wm::MOUSE_INPUT m){
    mpos = m.pos;
    if(m.btn == wm::MOUSE_BTN::M_SCRL_UP){
        playlist_cursor_offset--;
        refresh_playlist();
    }
    else if(m.btn == wm::MOUSE_BTN::M_SCRL_DOWN){
        playlist_cursor_offset++;
        refresh_playlist();
    }

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
    if(UIelements::toggle.space.inside(m.pos) != UIelements::toggle_hover){
        UIelements::toggle_hover = UIelements::toggle.space.inside(m.pos);
        refresh_play_button();
    }

}


void handle_input(int ch){
    //std::cout<< std::hex <<  std::setw(4*2) << ch;
    
    if(is_mouse(ch)){
        handle_mouse(wm::parse_mouse(ch));
        return;
    }
    
    auto c = (char)ch;
    if(c == 'n'){
        load_file(pl.next());
        
    }
}

std::chrono::duration sleep_for = std::chrono::milliseconds(1000/10);
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
        refresh_time_played();
        refresh_playbar();
        std::this_thread::sleep_for(sleep_for);
    }
    
}

int main(int argc, char const *argv[])
{
    init(argc, argv);
    refresh_all();
    std::thread thr(refrehs_thread);
    while (true) {
        if(wm::resize_event){
            handle_resize();
        }
        in_getch= true;
        int ch = wm::getch();
        in_getch= false;
        mv(0,0);
        handle_input(ch);
        if(ch == (int)'q'){
            clear_all()
            break;
        }
    }
    refrehs_thread_alive = false;
    deinit();
    thr.join();
    return 0;
}

