/*
#include "lib/audio/audio.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/core.hpp"
#include <cstdlib>
#include <iostream>

audio::Playlist pl;

wm::Element current_file;
wm::Element input_field;

wm::Element playlist;
wm::Element cover_art;
wm::Element playbar;

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
} // namespace UIelements


bool cover_valid = false;
std::string covert_img_path;

bool playlist_filename_only = true;
bool title_filename_only = true;

void load_file(std::string filepath){
    cover_valid = false;
    audio_vlc::load(filepath.c_str());
    set_title(
        (title_filename_only ? path::filename(filepath) : filepath).c_str()
    );
    audio_vlc::play();
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
}
void deinit() {
    audio_vlc::stop();
    audio_vlc::deinit();
    wm::deinit();
}
void csig(int sig){
    exit(0);
}

void init(int argc, char const *argv[]){
    //signals
    atexit(deinit);
    signal(SIGINT, csig);

    //window thingies
    wm::init();
    refresh_element_sizes();

    //playlist
    const char *filename = argc > 2 ? argv[2] : "playlist.pls";
    auto seek_to = pl.use(filename);
    //audio server
    audio_vlc::init();
    load_file(pl.current().c_str());
    audio_vlc::seek::abs(std::chrono::seconds(seek_to));
}


void handle_input(int ch){
    std::cout<< std::hex << ch;
}

*/
#include "lib/audio/sfml.hpp"
#include <chrono>
#include <iostream>

int main() {
    // Create an instance of sf::SoundBuffer to hold the audio data
    auto s = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    audio::load("stardust.mp3");
    auto c = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::cout << "Loading took: " << c-s << " nanoseconds\n";
    audio::control::play();

    getchar();

    audio::control::pause();

    return 0;
}
