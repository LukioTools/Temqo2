#include "lib/audio/playlist.hpp"
#include "lib/audio/extract_img.hpp"
#include "lib/audio/sfml.hpp"
#include "lib/cfg/config.hpp"
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
#include "lib/wm/space.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <regex>
#include <string>
#include <thread>
#include <valarray>
#include <variant>
#include <vector>
#include "custom/time_played.hpp"
#include "custom/button_array.hpp"


#define chartype int
#define cbtype void (*)()

#define mpair(k, v) std::make_pair(k, v)
#define clamp_1(val) (val == 0) ? 1 : val

audio::Playlist pl;

enum INPUT_MODE : char
{
    DEFAULT = 'D',
    SEARCH = 'S',
    COMMAND = 'C',
} current_mode = DEFAULT;

wm::Position mpos = {0, 0};
std::string input;

std::mutex rendering;

float volume_shift = 5.f;
float volume_reset = 100.f;
bool exit_mainloop = false;

ENUM(layout_t, unsigned char, HORIZONTAL, VERTICAL);
layout_t layout = layout_t::HORIZONTAL; 

// COLORS
cfg::RGB warning_color_fg = {150, 60, 60};
cfg::RGB warning_color_bg = {0, 0, 0};
cfg::RGB hilight_color_fg = {20, 30, 20};
cfg::RGB hilight_color_bg = {200, 200, 200};
cfg::RGB hover_color_fg = {222, 222, 222};
cfg::RGB hover_color_bg = {60, 60, 60};
cfg::RGB progress_bar_color_played_fg = {255, 0, 0};
cfg::RGB progress_bar_color_played_bg = {0, 0, 0};
cfg::RGB progress_bar_color_remaining_fg = {200, 200, 200};
cfg::RGB progress_bar_color_remaining_bg = {0, 0, 0};
cfg::RGB progress_bar_color_cursor_fg = {255, 255, 255};
cfg::RGB progress_bar_color_cursor_bg = {0, 0, 0};
cfg::RGB border_color_fg = {193, 119, 1};
cfg::RGB border_color_bg = {0, 0, 0};

// char/strings
std::string progres_bar_char_first = "├";
std::string progres_bar_char_center = "─";
std::string progres_bar_char_last = "┤";
std::string progres_bar_char_cursor = "•";

bool cover_art_override = false;
std::string cover_art_img_placeholder = "placeholder.png";
std::string playlist_file = "playlist.pls";

std::string cfg_path = "temqo.cfg";

// ease of use
int arrowkey_scroll_sensitivity = 1;
int mouse_scroll_sensitivity = 3;

wm::Element current_file;
wm::Element input_field;

wm::Element playlist;
wm::Element cover_art;

void refresh_time_played_default();
void refresh_time_played_remaining();

void load_file(std::string);
void refresh_playlist();

TimePlayed tp(
    {
        {11, refresh_time_played_default},
        {12, refresh_time_played_remaining},
    });

bool playback_control_inside = false;
bool looping = false;

std::string prev_char = "⏮";
std::string next_char = "⏭";
std::string shuffle_char = "⤮";
std::string sorted_char = "⇉";
std::string loop_on_char = "🗘";
std::string loop_off_char = "🡢";
std::string toggle_playing_char = "▶";
std::string toggle_stopped_char = "⏸";

ENUM(IDS, unsigned int,
     NO_ID,
     PREV,
     NEXT,
     SHUFFLE,
     TOGGLE,
     LOOP,
     VOLUME);

ENUM(GROUPS, short,
     UNGROUPED,
     MEDIA_CONTROLS,
     VOLUME);

ButtonArrayElement prev = {
    [](bool inside, wm::MOUSE_INPUT m)
    {
        if (inside)
        {
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg) << prev_char << " " << attr_reset;
            if (m.btn == wm::MOUSE_BTN::M_LEFT)
            {
                if (!pl.empty())
                {
                    load_file(pl.prev());
                }
            }
        }
        else
            std::cout << prev_char << " ";
    },
    2,
    GROUPS::MEDIA_CONTROLS,
    IDS::PREV};

ButtonArrayElement next = {
    [](bool inside, wm::MOUSE_INPUT m)
    {
        if (inside)
        {
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg) << next_char << " " << attr_reset;

            if (m.btn == wm::MOUSE_BTN::M_LEFT)
            {
                if (!pl.empty())
                {
                    load_file(pl.next());
                }
            }
        }
        else
            std::cout << next_char << " ";
    },
    2,
    GROUPS::MEDIA_CONTROLS,
    IDS::NEXT};

ButtonArrayElement shuffle = {
    [](bool inside, wm::MOUSE_INPUT m)
    {
        auto out_char = (pl.sorted() ? sorted_char : shuffle_char);
        if (inside)
        {
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg) << out_char << " " << attr_reset;

            if (m.btn == wm::MOUSE_BTN::M_LEFT)
            {
                if (pl.sorted())
                {
                    pl.new_seed();
                    pl.shuffle();
                }
                else
                    pl.sort();
                refresh_playlist();
            }
        }
        else
            std::cout << out_char << " ";
    },
    2,
    GROUPS::MEDIA_CONTROLS,
    IDS::SHUFFLE};

ButtonArrayElement toggle = {
    // playbutton
    [](bool inside, wm::MOUSE_INPUT m)
    {
        if (inside)
        {
            if (m.btn == wm::MOUSE_BTN::M_LEFT && !pl.empty())
            {
                audio::control::toggle();
            }

            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg);
        }
        auto ch = audio::playing() ? toggle_playing_char : toggle_stopped_char;
        std::cout << ch << " " << attr_reset;
    },
    2,
    1,
    4,
};

ButtonArrayElement loop = {
    [](bool inside, wm::MOUSE_INPUT m)
    {
        auto &use_char = looping ? loop_on_char : loop_off_char;
        if (inside)
        {
            std::cout << color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg) << use_char << " " << attr_reset;
            if (m.btn == wm::MOUSE_BTN::M_LEFT)
            {
                // loop one song
                looping = !looping;
            }
        }
        else
        {
            std::cout << use_char << " " << attr_reset;
        }
    },
    2,
    GROUPS::MEDIA_CONTROLS,
    IDS::LOOP};

ButtonArrayElement volume = {
    [](bool inside, wm::MOUSE_INPUT m)
    {
        // functionality
        if (inside)
        {
            use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg));

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

        // drawing
        int vol = std::abs((int)audio::volume::get());
        vol = vol > 999 ? 999 : vol;
        std::cout << std::setfill('0') << std::setw(3) << vol << '%' << ' ';

        if (inside)
        {
            use_attr(attr_reset);
        }
    },
    5,
    GROUPS::VOLUME,
    IDS::VOLUME};


std::vector<ButtonArrayElement> bae_elements({
    prev,
    toggle,
    next,
    shuffle,
    loop,
    volume,
});

struct BAE_NAME_TO_INDEX_T
{
    std::vector<std::regex> regexses = std::vector<std::regex>({
        std::regex("^ *previ?o?u?s? *$"),
        std::regex("^ *to?gg?le? *$"),
        std::regex("^ *next *$"),
        std::regex("^ *shuff?l?e? *$"),
        std::regex("^ *loop *$"),
        std::regex("^ *volu?m?e? *$"),
    });

    std::optional<size_t> find_index(std::string str){
        for(size_t i = 0; i < regexses.size(); i++){
            if(std::regex_match( str, regexses[i]))
                return i;
        }
        return std::nullopt;
    }
} bae_name_to_index;




//use default
ButtonArray playback_control(bae_elements);

namespace UIelements
{
    int settings_alloc = 1;
    // int volume_alloc = 4;
    //  int toggle_alloc = 1;
    bool settings_hover = false;
    // bool volume_hover = false;
    //  bool toggle_hover = false;
    wm::Element settings;
    // wm::Element volume;
    //  wm::Element toggle;
    /// @brief alloc rest of the ui space
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

size_t playlist_clamp(long i)
{
    if (pl.empty())
        return 0;
    while (i < 0)
    {
        i += pl.files.size();
    }
    size_t e = static_cast<size_t>(i);
    if (e >= pl.files.size())
    {
        e = e % pl.files.size();
    }
    return e;
}
size_t playlist_st_index()
{
    return playlist_clamp(playlist_display_offset);
}
size_t playlist_last_index()
{
    return playlist_clamp(playlist_display_offset + playlist.wSpace().h);
}
wm::SPLICE_TYPE playlist_clip = wm::SPLICE_TYPE::BEGIN_CUT;
wm::PAD_TYPE playlist_pad = wm::PAD_TYPE::PAD_RIGHT;

void refresh_configuration()
{
    cfg::parse(cfg_path);
    wm::resize_event = true;
}

void refresh_playlist()
{
    if (pl.empty())
        goto draw_boxes;
    if (playlist.aSpace().width() < 5 || playlist.aSpace().height() < 5)
        goto draw_boxes;
    // display the elements
    {
        auto s = playlist.wSpace();
        playlist_cursor_offset = playlist_clamp(playlist_cursor_offset);
        for (size_t index = 0; index <= s.h; index++)
        {
            auto i = playlist_clamp(playlist_display_offset + index);
            mv(s.x, s.y + index);
            auto str = pl[i];
            if (playlist_filename_only)
                str = path::filename(str);
            auto i_str = std::to_string(i) + ' ';
            wm::clip(str, s.width() - i_str.length(), playlist_clip);
            wm::pad(str, s.width() - i_str.length(), playlist_pad);

            str = i_str + str;

            if (i == static_cast<unsigned int>(playlist_cursor_offset))
            {
                use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg));
            }
            if (i == pl.current_index)
            {
                use_attr(color_bg_rgb(hilight_color_bg) << color_fg_rgb(hilight_color_fg));
            }

            mv(s.x, s.y + index);
            std::cout << str << attr_reset;
        }
    }
// draw the borders
draw_boxes:
{
    if(layout == layout_t::HORIZONTAL){
        auto s = playlist.space;
        std::string b;
        for (size_t i = 0; i < s.width(); i++)
        {
            b += "─";
        }
        use_attr(color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg));
        mv(s.x, s.y);
        std::cout << b;
        mv(s.x, s.y + s.h);
        std::cout << b << attr_reset;
    }
    else if(layout == layout_t::VERTICAL){
        auto s = playlist.space;
        auto e = s.end();
        std::string b;
        for (size_t i = 0; i < s.width(); i++)
        {
            b += "─";
        }
        use_attr(color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg));
        mv(s.x, s.y);
        std::cout << b;
        mv(s.x, e.y);
        std::cout << b << attr_reset;
    }
    
}
}

void refresh_display_offset()
{
    auto idx = playlist_clamp(playlist_cursor_offset);
    auto fst = playlist_st_index();
    auto lst = playlist_last_index();
    if (idx < fst)
    {
        playlist_display_offset -= fst - idx;
    }
    if (idx > lst)
    {
        playlist_display_offset -= lst - idx;
    }
}

void refresh_coverart_img()
{
    if (cover_art_override || pl.empty())
    {
        covert_img_path = cover_art_img_placeholder;
        cover_file_valid = true;
        return;
    }
    auto res = audio::extra::extractAlbumCoverTo(pl.current(), TMP_OUT);
    covert_img_path = (res == 0) ? TMP_OUT : cover_art_img_placeholder;
    cover_file_valid = true;
}

void refresh_coverart()
{
    cover_art_valid = true;
    use_attr(color_bg_rgb(border_color_bg) << color_fg_rgb(border_color_fg));
    if(layout == layout_t::HORIZONTAL)
        cover_art.space.box("─", "─", nullptr, "│", "┬", nullptr, "┴", nullptr);
    else if(layout == layout_t::VERTICAL)
        cover_art.space.box("─", "─", nullptr, nullptr, "─", "─", "─", "─");

    use_attr(attr_reset);
    // dont waste time
    if (cover_art.aSpace().width() < 3 || cover_art.aSpace().height() < 3)
        return;
    if (!cover_file_valid)
    {
        refresh_coverart_img();
    }
    if (covert_img_path.length() == 0)
    {
        return;
    }
    auto s = cover_art.wSpace();
    ascii_img::load_image_t *cover_ansi = audio::extra::getImg(covert_img_path, s.width(), s.height() + 1);

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
    return;
}

void refresh_currently_playing()
{
    std::string c = currently_playing_filename_only ? path::filename(pl.current()) : pl.current();
    auto s = current_file.aSpace();
    // current_file.space.fill("?");
    wm::clip(c, s.width(), wm::SPLICE_TYPE::BEGIN_DOTS);
    wm::pad(c, s.width(), wm::PAD_TYPE::PAD_CENTER);
    mv(s.x, s.y);
    std::cout << c;
}
void refresh_settings()
{
    auto pe = UIelements::settings.space;
    mv(pe.x, pe.y);
    auto inside = UIelements::settings_hover;

    if (inside)
    {
        use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout << "⚙ ";
    if (inside)
    {
        use_attr(attr_reset);
    }
}

#define leading_zero(n) ((n < 10) ? "0" : "") << n
/// alloc 11
void refresh_time_played_default()
{
    wm::Space s = tp;
    mv(s.x, s.y);
    auto ps = audio::position::get<std::ratio<1, 1>>().count();
    auto ts = audio::duration::get<std::ratio<1, 1>>().count();

    auto pm = ps / 60;
    auto tm = ts / 60;

    auto psec = ps - pm * 60;
    auto tsec = ts - tm * 60;

    if (tp.hover)
    {
        use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout
        << leading_zero(pm) << ':' << leading_zero(psec)
        << '/'
        << leading_zero(tm) << ':' << leading_zero(tsec);
    if (tp.hover)
    {
        use_attr(attr_reset);
    }
}
/// alloc 12?
void refresh_time_played_remaining()
{
    wm::Space s = tp;
    mv(s.x, s.y);
    auto ps = audio::position::get<std::ratio<1, 1>>().count();
    auto ts = audio::duration::get<std::ratio<1, 1>>().count();
    auto diff = ts - ps; // difference in seconds
    // abs time
    auto tm = ts / 60;
    auto tsec = ts - tm * 60;

    auto totaldiffminutes = diff / 60;
    auto totaldiffseconds = diff - (totaldiffminutes * 60);

    if (tp.hover)
    {
        use_attr(color_bg_rgb(hover_color_bg) << color_fg_rgb(hover_color_fg));
    }
    std::cout
        << '-' << leading_zero(totaldiffminutes) << ':' << leading_zero(totaldiffseconds)
        << '/'
        << leading_zero(tm) << ':' << leading_zero(tsec);
    if (tp.hover)
    {
        use_attr(attr_reset);
    }
}

#undef leading_zero

void refresh_time_played()
{
    auto e = tp.current();
    if (e.callback)
    {
        e.callback();
    }
}

void refresh_controls()
{
    playback_control.draw({mpos, wm::MOUSE_BTN::M_NONE, true});
}

void refresh_controls(wm::MOUSE_INPUT m)
{
    playback_control.draw(m);
}

void refresh_UIelements()
{
    refresh_settings();
    refresh_time_played();
    // refresh_volume();
    //  refresh_play_button();
    refresh_controls();
}
void refresh_playbar()
{
    if (current_mode == INPUT_MODE::DEFAULT)
    {
        auto d = audio::position::get_d();
        auto s = UIelements::playbar.space;
        auto idx = static_cast<unsigned int>(s.w * d);
        mv(s.x, s.y);
        use_attr(color_bg_rgb(progress_bar_color_played_bg) << color_fg_rgb(progress_bar_color_played_fg));
        for (size_t i = 0; i < s.width(); i++)
        {
            if (i == idx)
            {
                use_attr(attr_reset << color_bg_rgb(progress_bar_color_cursor_bg) << color_fg_rgb(progress_bar_color_cursor_fg));
                std::cout << progres_bar_char_cursor;
                use_attr(attr_reset << color_bg_rgb(progress_bar_color_remaining_bg) << color_fg_rgb(progress_bar_color_remaining_fg));
            }
            else if (i == 0)
            {
                std::cout << progres_bar_char_first;
            }
            else if (i == static_cast<unsigned long>(s.width() - 1))
            {
                std::cout << progres_bar_char_last;
            }
            else
            {
                std::cout << progres_bar_char_center;
            }
        }
        use_attr(attr_reset);
    }
    else if (current_mode == INPUT_MODE::COMMAND)
    {
        auto s = UIelements::playbar.space;
        auto str = ':' + input;
        wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
        mv(s.x, s.y);
        std::cout << str;
    }
    else if (current_mode == INPUT_MODE::SEARCH)
    {
        auto s = UIelements::playbar.space;
        auto str = '/' + input;
        wm::clip(str, s.w, wm::SPLICE_TYPE::BEGIN_DOTS);
        wm::pad(str, s.w, wm::PAD_TYPE::PAD_RIGHT);
        mv(s.x, s.y);
        std::cout << str;
    }
}
void refresh_element_sizes()
{
    current_file.space = wm::Space(0, 0, wm::WIDTH, 0);
    input_field.space = wm::Space(0, wm::HEIGHT, wm::WIDTH, 0);

    if(layout == layout_t::HORIZONTAL){
        playlist.space = wm::Space(0, 1, wm::WIDTH * playlist_width, wm::HEIGHT - 2);
        playlist.pad = {1, 1, 0, 1};
        cover_art.space = wm::Space(playlist.space.w, 1, wm::WIDTH - playlist.space.w + 1, wm::HEIGHT - 2);
        cover_art.pad = {1, 1, 1, 0};
    }
    else if (layout == layout_t::VERTICAL) {
        log_t << "VERTICAL" << std::endl;
        playlist.space = wm::Space(0, 1, wm::WIDTH, (wm::HEIGHT - 2) * playlist_width);
        playlist.pad = {1, 1, 0, 0};
        cover_art.space = wm::Space(0, playlist.space.h+1, wm::WIDTH, (wm::HEIGHT - 2)-playlist.space.h);
        cover_art.pad = {1, 1, 0, 0};
    }
    

    // order is important
    UIelements::settings.space = wm::Space(wm::WIDTH - UIelements::settings_alloc, wm::HEIGHT, UIelements::settings_alloc, 0);
    tp = wm::Space(UIelements::settings.space.x - 1 - tp.current().alloc_size, wm::HEIGHT, tp.current().alloc_size, 0);

    playback_control.pos = {static_cast<unsigned short>(tp.space.x - playback_control.width()), static_cast<unsigned short>(wm::HEIGHT)};
    UIelements::playbar.space = wm::Space(0, wm::HEIGHT, playback_control.pos.x - 1, 0);
}
// veri expensiv
void refresh_all()
{
    clear_all();
    use_attr(cursor_invisible);
    refresh_element_sizes();
    refresh_currently_playing();
    refresh_playlist();
    refresh_UIelements();
    refresh_playbar();
    try
    {
        refresh_coverart();
    }
    catch (...)
    {
    }
}

void load_file(std::string filepath)
{
    if (filepath == "")
    {
        return;
    }

    cover_file_valid = false;
    cover_art_valid = false;
    std::thread thr(audio::load, filepath);

    set_title(
        (title_filename_only ? path::filename(filepath) : filepath).c_str());
    playlist_cursor_offset = pl.current_index;
    refresh_display_offset();

    refresh_currently_playing();
    refresh_playlist();

    thr.join();
    audio::control::play();
    refresh_playbar();
    refresh_UIelements();
}

void deinit()
{
    audio::control::pause();
    wm::deinit();
}
void csig(int sig)
{
    audio::control::pause();
    wm::deinit();
    exit(0);
}

void handle_resize()
{
    wm::resize_event = false;
    cover_art_valid = false;
    refresh_all();
}

bool resize_drag = false;
wm::Position drag_start = {0, 0};

void handle_mouse(wm::MOUSE_INPUT m)
{
    mpos = m.pos;

    if (UIelements::settings.space.inside(m.pos) != UIelements::settings_hover)
    {
        UIelements::settings_hover = UIelements::settings.space.inside(m.pos);
        refresh_settings();
    }
    if (tp.space.inside(m.pos) != tp.hover)
    {
        tp.hover = !tp.hover;
        refresh_time_played();
    }

    if (playback_control.inside(m.pos) || playback_control.inside(m.pos) != playback_control_inside)
    {
        playback_control_inside = playback_control.inside(m.pos);
        refresh_controls(m);
    }
    if (UIelements::playbar.space.inside(m.pos) && m.btn == wm::MOUSE_BTN::M_LEFT)
    {
        auto delta = static_cast<double>(m.pos.x) / static_cast<double>(UIelements::playbar.space.width());
        audio::seek::abs(delta);
        refresh_time_played();
        refresh_playbar();
    }
    if (tp.space.inside(m.pos) && m.btn == wm::MOUSE_BTN::M_LEFT)
    {
        tp.next();
        mv(0, tp.space.y);
        clear_row();
        refresh_element_sizes();
        refresh_UIelements();
        refresh_playbar();
    }

    if (playlist.wSpace().inside(m.pos))
    {
        if (m.btn == wm::MOUSE_BTN::M_SCRL_UP)
        {
            playlist_cursor_offset -= mouse_scroll_sensitivity;
            refresh_display_offset();
        }
        else if (m.btn == wm::MOUSE_BTN::M_SCRL_DOWN)
        {
            playlist_cursor_offset += mouse_scroll_sensitivity;
            refresh_display_offset();
        }
        else
        {
            auto s = playlist.wSpace();
            auto i = m.pos.y - s.y;
            playlist_cursor_offset = playlist_clamp(playlist_display_offset + i);
            refresh_display_offset();
            if (m.btn == wm::MOUSE_BTN::M_LEFT)
            {
                if (playlist.aSpace().width() < 5 || playlist.aSpace().height() < 5)
                {
                }
                else
                {
                    pl.current_index = playlist_cursor_offset;
                    if (!pl.empty())
                        load_file(pl.current());
                }
            }
        }

        refresh_playlist();
    }

    if ((playlist.aSpace().end().x == m.pos.x && m.btn == wm::MOUSE_BTN::M_LEFT && layout == layout_t::HORIZONTAL) 
        || (playlist.aSpace().end().y == m.pos.y && m.btn == wm::MOUSE_BTN::M_LEFT && layout == layout_t::VERTICAL))
    {
        resize_drag = true;
    }
    if (resize_drag && m.btn == wm::MOUSE_BTN::M_RELEASE)
    {
        if(layout == layout_t::HORIZONTAL)
            playlist_width = static_cast<double>(m.pos.x) / static_cast<double>(wm::WIDTH); // its that simple
        else if(layout == layout_t::VERTICAL){
            log_t << m.pos.y << ':' <<  wm::HEIGHT-2 << std::endl;
            playlist_width = static_cast<double>(m.pos.y) / static_cast<double>(wm::HEIGHT); // its that simple
            log_t << playlist_width << std::endl;
        }
        if(playlist_width > 1) playlist_width = 1;
        if(playlist_width < 0) playlist_width = 0;

        wm::resize_event = true;
        resize_drag = false;
    }
}

void handle_arrow_key(wm::KEY k)
{
    switch (k)
    {
    case wm::K_UP:
        playlist_cursor_offset -= arrowkey_scroll_sensitivity;
        refresh_display_offset();
        refresh_playlist();
        break;
    case wm::K_DOWN:
        playlist_cursor_offset += arrowkey_scroll_sensitivity;
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

struct Command
{
    std::regex rgx;
    void (*cb)(const std::string &);
};
namespace hcr
{
    std::regex add_to_playlist("^p?l?add .*$");
    std::regex use_playlist("^p?l?use .*$");
    std::regex save_playlist("^p?l?save *$");
    std::regex clear_playlist("^p?l?cle?a?r_?p?l? .*$");
    std::regex saveto_playlist("^p?l?saveto .*$");
    std::regex use_config("^useco?n?fi?g .*$");
    std::regex goto_index("^got?o? .*$");
    std::regex refresh_config("^co?n?fi?grefr?e?s?h? *$");
} // namespace hcr

std::vector<Command> commands(std::initializer_list<Command>{
    {hcr::add_to_playlist, [](const std::string &)
     { pl.add(input.substr(input.find_first_of(' ') + 1)); }},
    {hcr::save_playlist, [](const std::string &)
     { pl.save(); }},
    {hcr::saveto_playlist, [](const std::string &)
     { pl.save(input.substr(input.find_first_of(' ') + 1)); }},
    {hcr::use_playlist, [](const std::string &)
     {pl.save();pl.use(input.substr(input.find_first_of(' ') + 1)); }},
    {hcr::use_config, [](const std::string &)
     {cfg_path = input.substr(input.find_first_of(' ') + 1);refresh_configuration(); }},
    {hcr::refresh_config, [](const std::string &)
     { refresh_configuration(); }},
    {hcr::goto_index, [](const std::string &)
     {playlist_cursor_offset = std::stoi(input.substr(input.find_first_of(' ') + 1));
            pl.current_index = playlist_clamp(playlist_cursor_offset);
            if(!pl.empty()){load_file(pl.current());} }},

});

void handle_command()
{
    if (input.length() == 0)
    {
        return;
    }
    try
    {
        for (Command c : commands)
        {
            if (!c.cb)
                continue;
            if (std::regex_match(input, c.rgx))
            {
                c.cb(input);
                break;
            }
        }
    }
    catch (...)
    {
        return;
    }
}
void handle_search()
{
    size_t index = std::string::npos;
    std::string out = pl.find_insensitive(input, &index);

    if (out.length() == 0 || index == std::string::npos)
    {
        return;
    }
    playlist_cursor_offset = index;
    refresh_display_offset();
    refresh_playlist();
}


namespace actions
{
    void exit_main() { exit_mainloop = true; }
    void next()
    {
        if (!pl.empty())
            load_file(pl.next());
    }
    void prev()
    {
        if (!pl.empty())
            load_file(pl.prev());
    }
    void ref_config() { refresh_configuration(); }
    void toggle_play()
    {
        if (!pl.empty())
        {
            audio::control::toggle();
            refresh_controls();
        }
    }
    void vol_up()
    {
        audio::volume::shift(volume_shift);
        playback_control.drawById(IDS::VOLUME, {mpos, wm::MOUSE_BTN::M_NONE, true});
    }
    void vol_down()
    {
        audio::volume::shift(-volume_shift);
        playback_control.drawById(IDS::VOLUME, {mpos, wm::MOUSE_BTN::M_NONE, true});
    }
    void select_cursor()
    {
        pl.current_index = playlist_clamp(playlist_cursor_offset);
        if (!pl.empty())
            load_file(pl.current());
    }
    void clear_input()
    {
        input = "";
    }
    void search()
    {
        input = "";
        current_mode = INPUT_MODE::SEARCH;
    }
    void command()
    {
        input = "";
        current_mode = INPUT_MODE::COMMAND;
    }
    void use_command()
    {
        handle_command();
        input = "";
        current_mode = DEFAULT;
    }
    void pop_from_input()
    {
        if (input.length() > 0)
        {
            input.pop_back();
        }
    }
    void add_to_input(char ch)
    {
        input += ch;
    }
    void select_cursor_clear()
    {
        select_cursor();
        clear_input();
        current_mode = DEFAULT;
    }
}

std::map<chartype, cbtype> default_mode_keys({
    mpair('q', actions::exit_main),
    mpair('n', actions::next),
    mpair('b', actions::prev),
    mpair('r', actions::ref_config),
    mpair('c', actions::toggle_play),
    mpair('+', actions::vol_up),
    mpair('-', actions::vol_down),
    mpair('\n', actions::select_cursor),
    mpair('/', actions::search),
    mpair(':', actions::command),
});

std::map<chartype, cbtype> command_mode_keys({
    mpair('\n', actions::use_command),
    mpair('\b', actions::pop_from_input),
    mpair(127, actions::pop_from_input),
});

std::map<chartype, cbtype> search_mode_keys({
    mpair('\n', actions::select_cursor_clear),
    mpair('\b', actions::pop_from_input),
    mpair(127, actions::pop_from_input),
});

// add the next found to the search shit
void handle_char(int ch)
{
    auto c = (char)ch;
    if (current_mode == DEFAULT) // yea the mode shit
    {
        if (auto cb = default_mode_keys[c])
            cb();
        else
        {
        } // idk
    }
    else if (current_mode == COMMAND)
    {
        if (auto cb = command_mode_keys[c])
            cb();
        else
            actions::add_to_input(c);
    }
    else if (current_mode == SEARCH)
    {
        if (auto cb = search_mode_keys[c])
            cb();
        else
            actions::add_to_input(c); // idk
        if (input.size())
        {
            handle_search();
        }
    }
    refresh_playbar();
}

void handle_input(int ch)
{
    if (ch == 0x1b1b)
    {
        input = "";
        current_mode = INPUT_MODE::DEFAULT;
        return;
    }
    if (is_mouse(ch))
    {
        handle_mouse(wm::parse_mouse(ch));
        return;
    }
    if (auto k = wm::is_key(ch))
    {
        handle_arrow_key(k);
        return;
    }
    if (ch > 0 && ch < 255)
    {
        handle_char(ch);
    }
}

std::chrono::duration sleep_for = std::chrono::milliseconds(100);
bool refrehs_thread_alive = true;

bool in_getch = false;
void refrehs_thread()
{
    while (refrehs_thread_alive)
    {
        if (in_getch)
        {
            std::lock_guard<std::mutex> lock(rendering);
            if (audio::stopped() && !pl.empty())
            {
                if (looping)
                    load_file(pl.current()); // seek to start if stopped
                else
                    load_file(pl.next());
            }
            if (wm::resize_event)
            {
                handle_resize();
            }
            if (!cover_art_valid)
            {
                try
                {
                    refresh_coverart();
                }
                catch (...)
                {
                }
            }
            refresh_time_played();
            refresh_playbar();
        }

        std::this_thread::sleep_for(sleep_for);
    }
}

void configuraton()
{
    ADD_CONFIG_INLINE("HilightColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        hilight_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        hilight_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    ADD_CONFIG_INLINE("WarnColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        warning_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        warning_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    ADD_CONFIG_INLINE("HoverColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        hover_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        hover_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    ADD_CONFIG_INLINE("BorderColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        border_color_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        border_color_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    /// Playlist
    ADD_CONFIG_INLINE("PlaylistFilenameOnly", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        playlist_filename_only = cfg::parse_bool(str); });
    ADD_CONFIG_INLINE("PlayingFilenameOnly", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        currently_playing_filename_only = cfg::parse_bool(str); });
    ADD_CONFIG_INLINE("TitleFilenameOnly", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        title_filename_only = cfg::parse_bool(str); });

    /// Progress Bar
    ADD_CONFIG_INLINE("ProgresBarPlayedColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_played_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_played_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    ADD_CONFIG_INLINE("ProgresBarRemainingColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_remaining_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_remaining_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });
    ADD_CONFIG_INLINE("ProgresBarCursorColor", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        progress_bar_color_cursor_fg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, 0    ));
        progress_bar_color_cursor_bg = cfg::parse_rgb(cfg::get_bracket_contents(str, &idx, idx+1)); });

    // i hope this works
    ADD_CONFIG_INLINE("ProgresBarChar", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        auto f      = cfg::get_bracket_contents(str, &idx, 0    );
        auto c      = cfg::get_bracket_contents(str, &idx, idx+1);
        auto l      = cfg::get_bracket_contents(str, &idx, idx+1);
        auto curs   = cfg::get_bracket_contents(str, &idx, idx+1);
        progres_bar_char_first = (f.length()>0) ? f : progres_bar_char_first;
        progres_bar_char_center = (c.length()>0) ? c : progres_bar_char_center;
        progres_bar_char_last = (l.length()>0) ? l : progres_bar_char_last;
        progres_bar_char_cursor = (curs.length()>0) ? curs : progres_bar_char_cursor; });

    /// CoverArt
    ADD_CONFIG_INLINE("CoverArt", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        enable_cover = cfg::parse_bool(str); });
    ADD_CONFIG_INLINE("CoverArtPlaceholder", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        str = cfg::get_bracket_contents(str);
        ////log_t << "new placeholder: " << str << std::endl;
        cover_file_valid = false;
        cover_art_valid = false;
        cover_art_img_placeholder = str; });
    ADD_CONFIG_INLINE("CoverArtOverride", [](const std::string &line) {});
    ADD_CONFIG_INLINE("Volume", [](const std::string &line)
                      {
        auto str = cfg::parse_inline(line);
        auto vol = std::stoi(str);
        
        log_t << "hell" << std::endl;
        audio::volume::set(vol); });

    ADD_CONFIG_INLINE("MediaControlChar", [](const std::string &line)
    {
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        auto play = cfg::get_bracket_contents(str, &idx, 0);
        auto stop = cfg::get_bracket_contents(str, &idx, idx+1);
        auto prev = cfg::get_bracket_contents(str, &idx, idx+1);
        auto next = cfg::get_bracket_contents(str, &idx, idx+1);
        auto shuffle = cfg::get_bracket_contents(str, &idx, idx+1);
        auto sorted = cfg::get_bracket_contents(str, &idx, idx+1);
        auto loop_on = cfg::get_bracket_contents(str, &idx, idx+1);
        auto loop_off = cfg::get_bracket_contents(str, &idx, idx+1);
        toggle_playing_char = (play.length()>0) ? play : toggle_playing_char;
        toggle_stopped_char = (stop.length()>0) ? stop : toggle_stopped_char;
        prev_char = (prev.length()>0) ? prev : prev_char;
        next_char = (next.length()>0) ? next : prev_char;
        shuffle_char = (shuffle.length()>0) ? shuffle : shuffle_char; 
        sorted_char = (sorted.length()>0) ? sorted : sorted_char; 
        loop_on_char = (loop_on.length()>0) ? loop_on : loop_on_char; 
        loop_off_char = (loop_off.length()>0) ? loop_off : loop_on_char; 
    });

    ADD_CONFIG_INLINE("PlaybackControlButtons", [](const std::string& line){
        auto str = cfg::parse_inline(line);
        size_t idx = 0;
        playback_control.clear();

        do
        {
            std::optional<std::string> pce = cfg::get_bracket_contents_conditional(str, &idx, idx==0 ? idx : idx+1);
            if(!pce.has_value())
                break;
            std::optional<size_t> index = bae_name_to_index.find_index(*pce);
            if(index.has_value() && index.value() < bae_elements.size()){
                playback_control.push_back(bae_elements[index.value()]);
            }
            else{
                try{
                    auto index = std::stoul(*pce);
                    if(index < bae_elements.size()){
                        playback_control.push_back(bae_elements[index]);
                    };
                }
                catch(...){}
            }            
            /* code */
        } while (idx != std::variant_npos);
        
    });

    ADD_CONFIG_INLINE("VerticalLayout", [](const std::string& line){
        auto str = cfg::parse_inline(line);
        bool vertical = cfg::parse_bool(str);
        if(vertical)
            layout = layout_t::VERTICAL;
        else
            layout = layout_t::HORIZONTAL;

    });

}
void init(int argc, char *const *argv)
{
    // signals
    atexit(deinit);
    signal(SIGINT, csig);

    // window thingies
    wm::init();
    use_attr(cursor_invisible);
    refresh_element_sizes();
    // config

    configuraton();
    refresh_configuration();

    auto seek_to = pl.use(playlist_file);

    if (!pl.empty())
    {
        //"⤭" shall be used to add a shuffle feature
        if (pl.seed)
        {
            pl.shuffle();
        }
        // audio server
        load_file(pl.current());
    }
    playlist_cursor_offset = pl.current_index;
    playlist_display_offset = pl.current_index;
    audio::seek::abs(std::chrono::seconds(seek_to));
}
// parse the argumenst
void input_args(int argc, char *const *argv)
{
    int option;
    // Process command-line options using getopt
    while ((option = getopt(argc, argv, "p:c:d:")) != -1)
    {
        switch (option)
        {
        case 'p':
            if (std::filesystem::exists(optarg))
                playlist_file = optarg;
            break;
        case 'c':
            if (std::filesystem::exists(optarg))
                cfg_path = optarg;
            break;
        case 'd':
            if (std::filesystem::exists(optarg))
            {
                log_t = std::ofstream(optarg);
                // log_t << argv[0] << " Logging..." << std::endl;
            }
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [-c config_file -p playlist_file -d debug_file/stream]" << std::endl;
            exit(1);
        }
    }
}

int main(int argc, char *const argv[])
{
    input_args(argc, argv);          // parse args
    init(argc, argv);                // initialize
    refresh_all();                   // refresh all
    std::thread thr(refrehs_thread); // exec thread
    while (!exit_mainloop)
    {
        // log_t << "Main lööp" << std::endl;
        if (wm::resize_event)
        {
            std::lock_guard<std::mutex> lock(rendering);
            if (wm::resize_event) // check that it is actually needed
                handle_resize();
        }
        in_getch = true;
        int ch = wm::getch();
        in_getch = false;
        {
            std::lock_guard<std::mutex> lock(rendering);
            handle_input(ch);
        }
    }
    refrehs_thread_alive = false;
    try
    {
        pl.save();
    }
    catch (...)
    {
    }
    deinit();
    thr.join();
    // log_t << "\ec" << std::endl;
    return 0;
}
