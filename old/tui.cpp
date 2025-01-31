#include "lib/ansi/ascii_img2.hpp"
#include "lib/audio/vlc.hpp"
#include "lib/audio/playlist.hpp"
#include "lib/audio/extract_img.hpp"
#include "lib/cfg/parsers.hpp"
#include "lib/path/filename.hpp"
#include "lib/wm/clip.hpp"
#include "lib/wm/core.hpp"
#include "lib/wm/def.hpp"
#include "lib/wm/element.hpp"
#include "lib/wm/getch.hpp"
#include "lib/wm/globals.hpp"
#include "lib/wm/mouse.hpp"
#include "lib/wm/position.hpp"
#include <cmath>
#include <codecvt>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <valarray>
#include <fstream>
#include <vlc/libvlc_media_player.h>
#include "lib/cfg/config.hpp"

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

//le cover art
std::string current_cover_image_path;
bool cover_valid = false;



inline void set_cover_refresh()
{
    cover_valid = false;
}

inline void invalidate_cover()
{
    current_cover_image_path = "";
    set_cover_refresh();
}



enum INPUT_MODE
{
    IM_PLAYBACK,
    IM_INPUT,
};

namespace wm
{
    std::string to_str(INPUT_MODE md){
        switch (md) {
            case IM_INPUT: return "INPUT";
            case IM_PLAYBACK: return "PLAYBACK";
            default:
                return "UNKNOWN";
        }
    }
} // namespace wm


INPUT_MODE mode = IM_PLAYBACK;
std::string input;



std::ofstream log_t("/dev/pts/5");
bool mainthread_waiting = false;
bool second_thread_waiting = false;

audio::Playlist p;

wm::Element *curplay_element = nullptr;
wm::Element *input_element = nullptr;

bool playlist_leftmost = true;
bool playlist_rightmost = false;
wm::Element *playlist_element = nullptr;

bool albumcover_leftmost = false;
bool albumcover_rightmost = true;
wm::Element *albumcover_element = nullptr;


int settings_button_alloc = 1;
wm::Element* settings_button = nullptr;
int time_alloc = 11;
wm::Element* time_button = nullptr;
int volume_button_alloc = 4;
wm::Element* volume_button = nullptr;
int playing_button_alloc = 1;
wm::Element* playing_button = nullptr;

wm::Element* progress_bar = nullptr;
wm::Element* text_input = nullptr;

int playlist_offset = 0;
int cursor_position = 0;
bool playlist_filename_only = true;
bool pls_dont_fill = true;
int volume_warn_treshold = 100;

bool enable_cover = true;
bool playlist_invalid = true;

size_t playlist_clamp(long i){
    while (i < 0)
    {
        i += p.files.size();
    }
    if (i >= p.files.size())
    {
        i = i % p.files.size();
    }
    return i;
}

int print_playlist()
{
    if(!p.files.size())
        return 1;
    log_t << "print_playlist" << std::endl;

    if (playlist_element == nullptr)
    {
        return -1;
    }
    if (playlist_element->not_valid())
    {
        return 0;
    }
    auto ws = playlist_element->wSpace();
    auto s = ws.start();
    auto e = ws.end();
    log_t << "print_playlist 2" << std::endl;

    auto i = playlist_clamp(playlist_offset);
    for (size_t y = s.y; y <= e.y; y++)
    {
        if (i >= p.files.size() || i < 0)
        {
            i = 0;
        }
        mv(s.x, y);
        if (i == p.current_index){
            use_attr(color_bg_rgb(hilight_color_bg) << color_fg_rgb(hilight_color_fg) << italic);
            //use_attr( color_bg_rgb(hilight_color_bg) << bold);
        }
        else if(i== cursor_position){
            use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg) << bold);
        }
        wm::sprintln(ws, std::to_string(i) + ": " + (playlist_filename_only ? path::filename(p[i]) : p[i]), wm::SPLICE_TYPE::END_DOTS);
        if (i == p.current_index || i == cursor_position){
            use_attr(attr_reset);
        }
        if (pls_dont_fill && y - s.y >= p.files.size() - 1)
        {
            break;
        }
        i++;
    }
    playlist_invalid = false;
    return 0;
}


wm::SPLICE_TYPE playing_clip = wm::BEGIN_DOTS;
wm::PAD_TYPE playing_pad = wm::PAD_CENTER;
bool playing_filename_only = true;


int print_playing(std::string song)
{
    log_t << "print_playing" << std::endl;

    if (!curplay_element)
        return -1;
    auto ws = curplay_element->aSpace(); // yeea im using absoluteSpace..... writeableSpace broki tehe :3 (mfw) https://media.tenor.com/qUprvC3gwB0AAAAd/taiho-shichau-zo-teehee.gif
    clear_row();
    if( playing_filename_only){
        song = path::filename(song);
    }
    boost::trim(song);
    wm::clip(song, ws.width(), playing_clip);
    wm::pad(song, ws.width(), playing_pad);


    std::cout << mv_str(ws.x, ws.y) << song;
    set_title(song.c_str());
    return 0;
}

int print_info(std::string info, unsigned int offset = 0)
{
    if (!input_element)
        return -1;

    auto ws = input_element->aSpace();
    auto e = ws.end();
    auto str = (mode == INPUT_MODE::IM_INPUT) ? input : '['+wm::to_str(mode)+']';
    wm::clip(str, (ws.width() - info.length() + offset), wm::SPLICE_TYPE::BEGIN_DOTS);
    
    std::cout << mv_str(ws.x,e.y) << clear_row_str <<  str << mv_str(e.x - info.length() + offset, e.y) << info;
    return 0;
}



#define leading_zero(n) ((n < 10) ? "0" : "") << n
void print_ui2(){

    auto vol = audio_vlc::volume::get();
    if(settings_button){
        mv(settings_button->space->x, settings_button->space->y);
        bool inside = settings_button->aSpace().inside(mpos);
        if(inside){
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg);
        }
        std::cout  << "⚙";

        if(inside){
            std::cout << attr_reset;
        }
    }
    if(time_button && audio_vlc::currentMedia){
        auto seconds = audio_vlc::played::get_s().count();
        auto minutes = seconds / 60;

        auto total_seconds = audio_vlc::lenght::get_s().count();
        auto total_minutes = total_seconds / 60;

        auto actual_seconds = seconds - (60 * minutes);
        auto actual_total_seconds = total_seconds - (60 * total_minutes);

        std::ostringstream ss;
        ss << leading_zero(minutes) << ':' << leading_zero(actual_seconds) << '/' << leading_zero(total_minutes) << ':' << leading_zero(actual_total_seconds);
        auto str = ss.str();
        wm::clip(str, time_button->aSpace().width(), wm::SPLICE_TYPE::END_CUT);
        
        mv(time_button->space->x, time_button->space->y);
        bool inside = time_button->aSpace().inside(mpos);
        if(inside){
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg);
        }
        std::cout << str;
        if(inside){
            std::cout << attr_reset;
        }
    }
    if(volume_button){
        auto vol = audio_vlc::volume::get();
        if(vol < 0){
            vol = -vol;
        }
        if(vol > 1000){
            vol = 999;
        }

        mv(volume_button->space->x, volume_button->space->y)
        auto volstr = std::to_string(vol);
        if(vol > volume_warn_treshold){
            std::cout << color_fg_rgb(warning_color_fg) << color_bg_rgb(warning_color_bg);
        }
        bool inside = volume_button->aSpace().inside(mpos);
        if(inside){
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg);
        }

        std::cout << ((vol > 99) ? std::to_string(vol) : '0'+((vol > 9) ? std::to_string(vol) : '0'+std::to_string(vol)) ) << '%';
        if(vol > volume_warn_treshold || inside){
            std::cout << attr_reset;
        }
    }
    if(playing_button){
        mv(playing_button->space->x, playing_button->space->y);

        bool inside = playing_button->aSpace().inside(mpos);
        if(inside){
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg);
        }

        std::cout << (audio_vlc::playing() ? "⏵" : "⏸");

        if(inside){
            std::cout << attr_reset;
        }
    }

    if(mode == INPUT_MODE::IM_INPUT && text_input){
        //create a copy
        auto str = input;
        auto s = text_input->aSpace();
        wm::clip(str, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str, s.width(), wm::PAD_RIGHT);
        mv(s.x, s.y);
        std::cout << str;  
    }
    if(mode == INPUT_MODE::IM_PLAYBACK && progress_bar && audio_vlc::mediaPlayer){
        auto s = progress_bar->wSpace();
        auto delta = audio_vlc::played::get();
        //auto seconds = audio_vlc::played::get_s().count();
        //auto total_seconds = audio_vlc::lenght::get_s().count();
        //auto _progress = static_cast<double>(seconds)/static_cast<double>(total_seconds);

        auto current_index = static_cast<size_t>(std::round(delta*s.width()));
        std::string str;
        str.append(color_bg_rgb_str(progress_bar_color_played_bg));
        str.append(color_fg_rgb_str(progress_bar_color_played_fg));
        for (size_t i = 0; i < s.width(); i++)
        {
            if(current_index == i){
                str.append(color_bg_rgb_str(progress_bar_color_remaining_bg));
                str.append(color_fg_rgb_str(progress_bar_color_remaining_fg));
                str.append(progres_bar_char_cursor);
            }
            else if(i==0){
                str.append(progres_bar_char_first);
            }
            else if(i == s.width()-1){
                str.append(progres_bar_char_last);
            }
            else{
                str.append(progres_bar_char_center);
            }
        }
        mv(s.x, s.y);
        std::cout << str << attr_reset;  
    }

}

#undef alzisst2ni

int refresh_cover()
{
    if (!enable_cover)
    {
        return 0;
    }
    if (!albumcover_element)
    {
        return -1;
    }
    if (albumcover_element->not_valid())
    {
        return 1;
    }
    if (p.files.size() == 0)
    {
        return 2;
    }
    auto res = audio::extra::extractAlbumCoverTo(p.current(), TMP_OUT);
    current_cover_image_path = (res == 0) ? TMP_OUT : "";
    return res;
}

int draw_album_cover()
{
    cover_valid = true;
    if (!enable_cover)
    {
        return 0;
    }
    if (!albumcover_element)
    {
        return -1;
    }
    if (albumcover_element->not_valid())
    {
        return 0;
    }
    if (current_cover_image_path == "")
    {
        if (refresh_cover())
        {
            return 1;
        }
        if (current_cover_image_path == "")
        {
            return -1;
        }
    }
    auto ws = albumcover_element->wSpace();
    auto cover_ansi = audio::extra::getImg(current_cover_image_path, ws.width(), ws.height() + 1);

    std::ostringstream out;
#define clamp_1(val) (val == 0) ? 1 : val
    for (size_t iy = 0; iy <= ws.height(); iy++)
    {
        out << mv_str(ws.x, ws.y + iy);
        for (size_t ix = 0; ix < ws.width(); ix++)
        {
            auto rgb = cover_ansi->get(ix + ws.width() * iy);

            out << color_bg_str(clamp_1(rgb.r), clamp_1(rgb.g), clamp_1(rgb.b)) << ' ' << attr_reset;
        }
    }
    delete cover_ansi;
    std::cout << out.str();
    return 0;
}

int playlist_width_offset = -3;

void refresh_elements()
{
    *curplay_element->space = wm::Space(0, 0, wm::WIDTH, 1);
    *input_element->space = wm::Space(0, wm::HEIGHT - 1, wm::WIDTH, 1);

    *playlist_element->space = wm::Space(0, 1, (wm::WIDTH / 2) + playlist_width_offset, wm::HEIGHT - 2);
    playlist_rightmost = playlist_element->space->end().x == wm::WIDTH;
    playlist_leftmost = playlist_element->space->x == 0;
    playlist_element->pad = {1, 1, static_cast<unsigned char>(playlist_leftmost ? 1 : 2), static_cast<unsigned char>(playlist_rightmost ? 1 : 2)};

    *albumcover_element->space = wm::Space(playlist_element->space->end().x + 1, 1, (wm::WIDTH / 2) - playlist_width_offset, wm::HEIGHT - 2);
    albumcover_rightmost = albumcover_element->space->end().x == wm::WIDTH;
    albumcover_leftmost = albumcover_element->space->x == 0;
    albumcover_element->pad = {1, 1, static_cast<unsigned char>(1 * !albumcover_leftmost), static_cast<unsigned char>(1 * !albumcover_rightmost)};


    *settings_button->space =   wm::Space(wm::WIDTH-settings_button_alloc,wm::HEIGHT,settings_button_alloc,0);
    *time_button->space     =   wm::Space(wm::WIDTH-settings_button_alloc-1-time_alloc,wm::HEIGHT,time_alloc,0);
    *volume_button->space   =   wm::Space(wm::WIDTH-settings_button_alloc-1-time_alloc-1-volume_button_alloc,wm::HEIGHT,volume_button_alloc,0);
    *playing_button->space  =   wm::Space(wm::WIDTH-settings_button_alloc-1-time_alloc-1-volume_button_alloc-1-playing_button_alloc,wm::HEIGHT,playing_button_alloc,0);

    *progress_bar->space    = wm::Space(0, wm::HEIGHT, (wm::WIDTH-1)-(wm::WIDTH-playing_button->space->x), 0);
    *text_input->space    = wm::Space(0, wm::HEIGHT, (wm::WIDTH-1)-playing_button->space->width(), 0);

}

void light_refresh()
{
    if (playlist_element)
    {
        if (!playlist_element->not_valid())
        {
            std::cout<< color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg);
            playlist_element->aSpace().box("─", "─", playlist_rightmost ? nullptr : "│", playlist_leftmost ? nullptr : "│", playlist_leftmost ? "─" : "┬", playlist_rightmost ? "─" : "┬", playlist_leftmost ? "─" : "┴", playlist_rightmost ? "─" : "┴");
            std::cout<< attr_reset;
        }
    }
    log_t << "aPlaylist" << std::endl;

    if (albumcover_element)
    {
        std::cout<< color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg);
        albumcover_element->aSpace().box("─", "─", nullptr, playlist_leftmost ? nullptr : "│", "─", "─", "─", "─");
        std::cout<< attr_reset;
        //auto ws = albumcover_element->wSpace();
        //mv(ws.x, ws.y);
    }
    log_t << "after albumcover_element" << std::endl;
    if (!cover_valid)
    {
        draw_album_cover();
    }
    log_t << "after Cover" << std::endl;
    if(playlist_invalid){
        print_playlist();
    }
    log_t << "after Playlist" << std::endl;
    //print_ui();
    print_ui2();
}

void force_refresh()
{
    clear_scr();
    refresh_elements();
    playlist_invalid = true;
    cover_valid = false;
    print_playing(p.current());
    log_t << "Light Refreh" << std::endl;
    light_refresh();
}

void init_elements()
{
    log_t << "init variables" << std::endl;
    curplay_element = new wm::Element(new wm::Space(0, 0, 0, 0), {0, 0, 0, 0});
    input_element = new wm::Element(new wm::Space(0, 0, 0, 0), {0, 0, 0, 0});
    playlist_element = new wm::Element(new wm::Space(0, 0, 0, 0), {1, 1, static_cast<unsigned char>(2 * !playlist_leftmost), static_cast<unsigned char>(2 * !playlist_rightmost)});
    albumcover_element = new wm::Element(new wm::Space(0, 0, 0, 0), {1, 1, static_cast<unsigned char>(1 * !albumcover_leftmost), static_cast<unsigned char>(1 * !albumcover_rightmost)});

    //ui elements
    settings_button = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});
    volume_button = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});
    playing_button = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});
    time_button = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});

    progress_bar = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});
    text_input = new wm::Element(new wm::Space(0,0,0,0), {0,0,0,0});
    refresh_elements();

}

#define del_el(el) if(el){delete el->space;delete el; el = nullptr;};
void deinit_elements()
{
    if (curplay_element)
    {
        delete curplay_element->space;
        delete curplay_element;
        curplay_element = nullptr;
    }

    if (input_element)
    {
        delete input_element->space;
        delete input_element;
        input_element = nullptr;
    }

    if (playlist_element)
    {
        delete playlist_element->space;
        delete playlist_element;
        playlist_element = nullptr;
    }

    del_el(settings_button);
    del_el(time_button);
    del_el(volume_button);
    del_el(playing_button);
    del_el(progress_bar);
    del_el(text_input);

}

void load_audio(std::string fn)
{
    cover_valid = false;

    auto filename = path::filename(fn);
    audio_vlc::load(fn.c_str());
    cursor_position = p.current_index;

    print_playing(filename);
    print_playlist();
    invalidate_cover();
}

void next_callback()
{
    auto next_song_path = p.next();
    load_audio(next_song_path);
}

void secondly(long)
{
    if (mainthread_waiting)
    {
        second_thread_waiting = true;
        if (wm::resize_event)
        {
            wm::resize_event = false;
            force_refresh();
        }
        //print_ui();
        print_ui2();
        std::cout.flush();
    }
    second_thread_waiting = false;
}

wm::Position drag_p_s = {0, 0};
wm::Position drag_p_e = {0, 0};
bool drag_b = false;
void drag_m(wm::MOUSE_INPUT m)
{
    playlist_invalid = true;

    if (!playlist_element)
    {
        return;
    }
    if (drag_p_s.x == playlist_element->aSpace().end().x)
    {
        playlist_width_offset -= drag_p_s.x - drag_p_e.x;
        force_refresh();
    }
}

int click(wm::MOUSE_INPUT m)
{
    // song switcher
    if (m.btn == wm::M_LEFT && playlist_element->wSpace().inside(m.pos))
    {
        playlist_invalid = true;

        if (!playlist_element)
        {
            return 0;
        }
        auto index = m.pos.y - playlist_element->wSpace().y;
        auto e = playlist_clamp(playlist_clamp(playlist_offset) + index);
        if (e != p.current_index)
        {
            p.current_index = e;
            /// playlist_offset = 0; // put cursor to the top
            load_audio(p.current());
        }
    }
    
    if(playing_button->aSpace().inside(m.pos)){
        audio_vlc::toggle_play();
    }

    if(progress_bar && progress_bar->wSpace().inside(m.pos) && audio_vlc::currentMedia){
        auto s = progress_bar->wSpace();
        auto delta = m.pos.x-s.x;
        auto delta_d = static_cast<double>(delta)/static_cast<double>(s.width());
        audio_vlc::seek::abs(delta_d);
    }
    return 0;
}

void handle_mouse(wm::MOUSE_INPUT m)
{
    mpos = m.pos;
    switch (m.btn)
    {
    case wm::MOUSE_BTN::M_SCRL_UP:
        if(playlist_element->wSpace().inside(m.pos)){
            playlist_offset-=mouse_scroll_sensitivity;
            playlist_invalid = true;
        }
        else if (volume_button->aSpace().inside(m.pos)){
            audio_vlc::volume::shift(1);
        }
        break;
    case wm::MOUSE_BTN::M_SCRL_DOWN:
        if(playlist_element->wSpace().inside(m.pos)){
            playlist_offset+=mouse_scroll_sensitivity;
            playlist_invalid = true;
        }
        else if (volume_button->aSpace().inside(m.pos)){
            audio_vlc::volume::shift(-1);
        }
        break;
    case wm::MOUSE_BTN::M_LEFT:
        drag_p_s = m.pos;
        click(m);
        break;
    case wm::MOUSE_BTN::M_LEFT_HILIGHT:
        drag_b = true;
        break;
    case wm::MOUSE_BTN::M_RELEASE:
        if (drag_b)
        {
            drag_p_e = m.pos;
            drag_m(m);
        }
        break;
    case wm::MOUSE_BTN::M_NONE:
        if(playlist_element && playlist_element->wSpace().inside(m.pos)){
            playlist_invalid = true;
            auto index = m.pos.y - playlist_element->wSpace().y;
            cursor_position = playlist_clamp(playlist_clamp(playlist_offset) + index);
        }
    default:
        break;
    }

    if (m.btn == wm::MOUSE_BTN::M_LEFT_HILIGHT)
    {
        drag_b = true;
    }
}


std::regex find_i("^/.*$");
std::regex find("^fi?n?d? .*$");
std::regex find_rgx("^re?ge?x .*$");
std::regex command("^:.*$");

std::string find_and_seek(bool play = false){
    playlist_invalid = true;
    size_t index = std::string::npos;
    std::string out;
    if(std::regex_match(input, find_i)){
        auto str = input.substr(1);
        out = p.find_insensitive(str, &index);
    }
    else if(std::regex_match(input, find_rgx)){
        auto str = input.substr(input.find_first_of(' '));
        out = p.frgx(std::regex(str), &index);
    }

    //seek
    if(out.length() == 0 || index == std::string::npos){
        return "";
    }
    cursor_position = index;
    playlist_offset = index;
    return out;
}

void handle_input(int c)
{
    char ch = static_cast<char>(c);
    if (c == 0x1b1b)
    {
        input = "";
        mode = INPUT_MODE::IM_PLAYBACK;
        return;
    }
    if(ch == '\b' || c == 127){
        if(input.length() > 0){
            input.pop_back();
            find_and_seek();
        }
        //halfway_process();
    }
    //[space, ~]
    else if (c > 0x1F && c < 0x7F)
    {
        input += static_cast<char>(c);
        find_and_seek();
    }
    else if (ch == '\n')
    { // process input
        //process_input();
        {
            auto str = p[cursor_position];
            if(str.length() != 0 ){
                p.current_index = cursor_position;
                load_audio(str);
            }
        }
        
        input = "";
        mode = INPUT_MODE::IM_PLAYBACK;
    }
    
}

void sigexit(int sig){
    //try
    //{
    //    //p.save();
    //}
    //catch (...)
    //{
    //    // idk man
    //    std::cout << "Save config not found\n";
    //}
    audio_vlc::deinit();
    deinit_elements();
    use_attr(disable_mouse(USE_MOUSE) << norm_buffer << cursor_visible);
    wm::deinit();
    exit(0);
}

void song_ended(){
    next_callback();
    light_refresh();
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
        playing_filename_only = cfg::parse_bool(str);
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
        if(vol > 200){
            vol = 2;
        }
        if(vol < -200){
            vol = -200;
        }
        audio_vlc::volume::set(vol);
    });
    
}

const char* config_path = "temqo.cfg";

int main(int argc, char const *argv[])
{
    log_t << "\ec" << std::endl;
    {
        const char *filename = argc > 2 ? argv[2] : "playlist.pls";
        audio_vlc::init();
        audio_vlc::on::end(song_ended);
        audio_vlc::on::time(secondly);
        
        clear_all();
        
        // std::cout << "loading file: " << filename << "\n";
        signal(SIGINT, sigexit);
        
        configuraton();
        cfg::parse(config_path);

        auto seek_seconds = p.use(filename);
        cursor_position = p.current_index;

        audio_vlc::load(p.current().c_str());
        audio_vlc::play();
        audio_vlc::seek::abs(static_cast<double>(seek_seconds)/audio_vlc::lenght::get_s_d());

        wm::init();
        init_elements();
    }
    log_t << "Hell is broken loose" << std::endl;
    
    playlist_offset = p.current_index;
    use_attr(cursor_invisible);
    log_t << "force refresh" << std::endl;
    force_refresh();
    while (true)
    {
        log_t << "GetChar" << std::endl;
        mainthread_waiting = true;
        auto c = wm::getch();
        mainthread_waiting = false;
        //log_t << "c:" << c << std::endl;
        if (c == RESIZE_EVENT)
        {
            force_refresh();
        }
        if (is_mouse(c))
        {
            auto m = wm::parse_mouse(c);
            handle_mouse(m);
        }

        if (auto key = wm::is_key(c))
        {
            switch (key)
            {
            case wm::KEY::K_RIGHT:
                audio_vlc::seek::rel(std::chrono::seconds(5));
                break;
            case wm::KEY::K_LEFT:
                audio_vlc::seek::rel(std::chrono::seconds(-5));
                break;
            case wm::KEY::K_UP:
                playlist_invalid = true;
                cursor_position-=arrowkey_scroll_sensitivity;
                {
                    auto i = playlist_clamp(playlist_offset);
                    if(i > cursor_position){
                        playlist_offset-=arrowkey_scroll_sensitivity;
                    }
                }

                break;
            case wm::KEY::K_DOWN:
                playlist_invalid = true;
                cursor_position+=arrowkey_scroll_sensitivity;
                {
                    auto i = playlist_clamp(playlist_offset);
                    if(i+ playlist_element->wSpace().h < cursor_position){
                        playlist_offset+=arrowkey_scroll_sensitivity;
                    }
                }
                break;
            default:
                break;
            }
        }

        switch (mode)
        {
        case INPUT_MODE::IM_INPUT:
            handle_input(c);
            break;
        case INPUT_MODE::IM_PLAYBACK:
        {
            switch (c)
            {
            case 'q':
                goto exit;
                break;
            case 'i':
                input = "";
                mode = INPUT_MODE::IM_INPUT;
                break;
            case 'h':
                // print help
                break;
            case 'f':
                playlist_filename_only = !playlist_filename_only;
                playlist_invalid = true;

                break;
            case 'a':
                p.sort();
                load_audio(p.current());
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
                audio_vlc::toggle_play();
                break;
            case '+':
                audio_vlc::volume::shift(10);
                break;
            case '-':
                audio_vlc::volume::shift(-10);
                break;
            case 'r':
                cfg::parse(config_path);
                force_refresh();
                break;
            case '/':
                input= "/";
                mode = INPUT_MODE::IM_INPUT;
                break;
            case ':':
                input= ":";
                mode = INPUT_MODE::IM_INPUT;
                break;
            case '\n':
                {
                    auto i = playlist_clamp(cursor_position);
                    if(i >= p.files.size()){
                        break;
                    }
                    p.current_index = i;
                    load_audio(p.current());    
                }
                
                break;
            default:
                break;
            }
        }
        }

        light_refresh();
    }
exit:
    try
    {
        p.save();
    }
    catch (...)
    {
        // idk man
        std::cout << "Save config not found\n";
    }
    use_attr(cursor_visible);
    audio_vlc::deinit();
    deinit_elements();
    wm::deinit();

    return 0;
}
