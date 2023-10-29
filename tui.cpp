#include "lib/ansi/ascii_img2.hpp"
#include "lib/audio/audio.hpp"
#include "lib/audio/audio_backend.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/core.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/element.hpp"
#include "lib/wm/getch.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/init.hpp"
#include "lib/wm/space.hpp"
#include <codecvt>
#include <iostream>
#include <sstream>
#include <string>

bool mainthread_waiting = true;

audio::Playlist p;

wm::Element* curplay_element = nullptr;
wm::Element* input_element = nullptr;

bool playlist_leftmost = true;
bool playlist_rightmost = true;
wm::Element* playlist_element = nullptr;

int print_playing(std::string song){
    if(!curplay_element)
        return -1;
    auto ws = curplay_element->aSpace(); //yeea im using absoluteSpace..... writeableSpace broki tehe :3 (mfw) https://media.tenor.com/qUprvC3gwB0AAAAd/taiho-shichau-zo-teehee.gif
    mv(ws.x, ws.y);
    clear_row();
    boost::trim(song);
    wm::sprintln(ws, song);
    set_title(song.c_str());
    return 0;
}

int print_info(std::string info, unsigned int offset = 0){
    if(!input_element)
        return -1;

    auto ws = input_element->aSpace();
    auto s = ws.start();
    auto e = ws.end();
    mv(e.x-info.length()+offset, e.y);
    clear_row();
    std::cout << info;
    return 0;
}

void refresh_elements(){
    *curplay_element->space = wm::Space(0,0,wm::WIDTH,1);
    *input_element->space = wm::Space(0, wm::HEIGHT-1, wm::WIDTH, 1);
    *playlist_element->space = wm::Space(0,1,wm::WIDTH, wm::HEIGHT-2);
}

void init_elements(){
    curplay_element = new wm::Element(new wm::Space(0,0,0,0), {0,1,0,0});
    input_element = new wm::Element(new wm::Space(0,0,0,0), {1,0,0,0});
    playlist_element = new wm::Element(new wm::Space(0,0,0,0), {1,1,static_cast<unsigned char>(2*!playlist_leftmost),static_cast<unsigned char>(2*!playlist_rightmost)});

    refresh_elements();
}

void deinit_elements(){
    if(curplay_element){
        delete curplay_element->space;
        delete curplay_element;
        curplay_element = nullptr;
    }
    
    if(input_element){
        delete input_element->space;
        delete input_element;
        input_element = nullptr;
    }

    if(playlist_element){
        delete playlist_element->space;
        delete playlist_element;
        playlist_element = nullptr;
    }
    
}

void load_audio(std::string fn){
    auto filename = path::filename(fn);
    audio::load_next(fn.c_str(), true);
    print_playing(filename);
}

void next_callback(){
    auto next_song_path = p.next();
    load_audio(next_song_path);
}

ascii_img::RGB<> warning_color = {150, 60, 60};

void print_ui(){
    std::ostringstream ss;
    auto vol = (int)(audio::volume.load()*100);
    ss 
        << (audio::playing ? "⏵" : "⏸")
        << ' '
        << ((vol > 100) ? color_fg_str(warning_color.r, warning_color.g, warning_color.b) : "")
        <<std::to_string(vol)
        << '%'
        << ((vol > 100) ? attr_reset : "");
        //add_a_leading_zero_if_is_shorter_than_2_numbers_int
#define alzisst2ni(number) ((number < 10) ? "0" : "") << number
    if(audio::curr){
        auto seconds = audio::framesRead/audio::curr->outputSampleRate;
        auto minutes = seconds/60;

        auto aaa = audio::seconds_in_current();
        auto aaa_minutes = aaa/60;

        ss << ' ' << alzisst2ni(minutes) << ':' << alzisst2ni(seconds - 60*minutes) << '/' << alzisst2ni(aaa_minutes) << ':' << alzisst2ni(aaa- 60*aaa_minutes);
    }

    print_info(ss.str(), 3+(std::string(((vol > 100) ? color_fg_str(warning_color.r, warning_color.g, warning_color.b) : ""))+std::string(((vol > 100) ? attr_reset : ""))).length());
    
}
#undef alzisst2ni

void secondly(void){
    if(mainthread_waiting){
        print_ui();
        std::cout.flush();
    }
        
    
}

int main(int argc, char const *argv[])
{
    {
        const char* filename = argc > 2 ? argv[2]:"playlist.pls";
        clear_all();
        //std::cout << "loading file: " << filename << "\n";
        auto seek_seconds = p.use(filename);
        audio::init(p.current().c_str());
        audio::songEndedCallback = next_callback;
        audio::song_played_secondly = secondly;
        audio::play();

        audio::seek_abs(std::chrono::seconds(seek_seconds));

        wm::init();
        init_elements();
        print_playing(p.current());
    }
    use_attr(cursor_invisible);
    while(true){
        mainthread_waiting = true;
        auto c = wm::getch();
        mainthread_waiting = false;
        if(c == RESIZE_EVENT){
            clear_scr();
            refresh_elements();
        }
        if(print_playing(path::filename(p.current()))){
            goto exit;
        }
        
        {
            curplay_element->aSpace().box(nullptr, "─", nullptr, nullptr, nullptr, nullptr, "─", "─");
            input_element->aSpace().box("─", nullptr, nullptr, nullptr, "─", "─", nullptr, nullptr);
        }
        if(playlist_element) {
            //playlist_element->aSpace().fill("#"); // ┬ // ┴
            playlist_element->aSpace().box(nullptr, nullptr, playlist_rightmost ? nullptr : "│" , playlist_leftmost ? nullptr : "│", playlist_leftmost ? "─" : "┬" , playlist_rightmost ? "─" : "┬", playlist_leftmost ? "─" : "┴" , playlist_rightmost ? "─" : "┴"  );
            //playlist_element->wSpace().fill("#");
        }
        //if(curplay_element) curplay_element->aSpace().fill("#");//.box("#", "─", "R", "L", "0", "1", "─", "─");
        print_ui();
        switch (c)
        {
        case 'q':
            goto exit;
            break;
        case 'i':
            //print info
            break;
        case 'h':
            //print help
            break;
        case 's':
            p.shuffle();
            load_audio(p.current());
            break;
        case 'n':
            next_callback();
            break;
        case 'b':
            load_audio(p.prev());
            break;
        case 'c':
            audio::playing ? audio::stop() : audio::play();
            print_ui();
            break;
        case '+':
            audio::vol_shift(.1f);
            print_ui();
            break;
        case '-':
            audio::vol_shift(-.1f);
            print_ui();
            break;
        default:
            break;
        }


    }
    exit:
    try{
        p.save();
    }
    catch(...){
        //idk man
        std::cout << "Save config not found\n";
    }
    use_attr(cursor_visible);
    audio::deinit();
    deinit_elements();
    wm::deinit();


    return 0;
}
