#include "lib/audio/playlist.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/clip.hpp"
#include "lib/wm/core.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/mouse.hpp"
#include "lib/wm/space.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>

audio::Playlist pl;

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
    wm::Element settings;
    wm::Element time_played;
    wm::Element volume;
    wm::Element toggle;
    /// @brief allo rest of the ui space
    wm::Element playbar;
} // namespace UIelements


bool cover_valid = false;
std::string covert_img_path;

bool playlist_filename_only = true;
bool currently_playing_filename_only = true;
bool title_filename_only = true;



void refresh_playlist(){
    playlist.space.fill("P");
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
    std::cout <<  "⚙ ";
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

    std::cout 
    << leading_zero(pm) << ':' << leading_zero(psec) 
    << '/' 
    << leading_zero(tm) << ':' << leading_zero(tsec);
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

    std::cout << std::setfill('0') << std::setw(s.w-1) << vol << "% ";
}

void refresh_play_button(){
    auto s = UIelements::toggle.space;
    mv(s.x,s.y);
    std::cout << std::setw(s.w-1) << (audio::playing() ? "⏵" : "⏸") << ' ';
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
    UIelements::playbar.space.fill("─");
}
void refresh_element_sizes(){
    current_file.space = wm::Space(0,0, wm::WIDTH, 0);
    input_field.space =  wm::Space(0, wm::HEIGHT-1, wm::WIDTH, 0);
    
    playlist.space = wm::Space(0,1, wm::WIDTH, wm::HEIGHT-2);
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
    audio::seek::abs(std::chrono::seconds(seek_to));
}
void handle_resize(){
    refresh_all();
    wm::resize_event = false;
}
void handle_mouse(wm::MOUSE_INPUT m){

}


void handle_input(int ch){
    //std::cout<< std::hex <<  std::setw(4*2) << ch;
    if(wm::resize_event){
        handle_resize();
    }
    if(is_mouse(ch)){
        handle_mouse(wm::parse_mouse(ch));
        return;
    }
    
    auto c = (char)ch;
    if(c == 'n'){
        load_file(pl.next());
        
    }
}

int main(int argc, char const *argv[])
{
    init(argc, argv);
    refresh_all();
    while (true) {
        int ch = wm::getch();
        mv(0,0);
        handle_input(ch);
        if(ch == (int)'q'){
            clear_all()
            break;
        }
    }

    deinit();
    return 0;
}

