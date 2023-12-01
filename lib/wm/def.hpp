
#pragma once
#define mv(x,y) std::cout << "\e["<< y+1 << ";"<< x+1 <<"H" ;
#define mv_str(x,y) "\e[" + std::to_string(y+1) + ";" + std::to_string(x+1) + "H" 
#define clear_all_no_mouse() std::cout << "\ec" ;
#define clear_all() std::cout << "\ec" << enable_mouse(USE_MOUSE) ;
#define clear_scr() std::cout << "\e[2J" ;
#define clear_row() std::cout << "\e[2K" ;
#define clear_row_str  "\e[2K"
#define clear_curs_eol() std::cout << "\e[0K" ;
#define clear_curs_sol() std::cout << "\e[1K" ;

#define alert '\a'

#define cursor_home "\e[H"
#define cursor_up(n)    ("\e["+std::to_string(n)+"A")
#define cursor_down(n)  ("\e["+std::to_string(n)+"B")
#define cursor_right(n) ("\e["+std::to_string(n)+"C")
#define cursor_left(n)  ("\e["+std::to_string(n)+"D")
#define cursor_up_scrl "\eM"
//ESC[#G 	                moves cursor to column #
#define cursor_to_column(n) ("\e[" + std::to_string(n) + "G")

#define cursor_save "\e7"
#define cursor_load "\e8"
#define cursor_save_sco "\e[s"
#define cursor_load_sco "\e[u"

//use charachter width
#define color_fg(r,g,b) "\e[38;2;"<< (int) r << ';'<< (int) g << ';' << (int) b << 'm'
#define color_bg(r,g,b) "\e[48;2;"<< (int) r << ';'<< (int) g << ';' << (int) b << 'm'

#define color_fg_rgb(rgb) "\e[38;2;"<< (int) rgb.r << ';'<< (int) rgb.g << ';' << (int) rgb.b << 'm'
#define color_bg_rgb(rgb) "\e[48;2;"<< (int) rgb.r << ';'<< (int) rgb.g << ';' << (int) rgb.b << 'm'

#define color_fg_str(r,g,b) ("\e[38;2;" + std::to_string((int) r) + ';' + std::to_string((int) g) + ';' + std::to_string((int) b) + 'm')
#define color_bg_str(r,g,b) ("\e[48;2;" + std::to_string((int) r) + ';' + std::to_string((int) g) + ';' + std::to_string((int) b) + 'm')

#define color_fg_rgb_str(rgb) ("\e[38;2;" + std::to_string((int) rgb.r) + ';' + std::to_string((int) rgb.g) + ';' + std::to_string( (int) rgb.b) + 'm')
#define color_bg_rgb_str(rgb) ("\e[48;2;" + std::to_string((int) rgb.r) + ';' + std::to_string((int) rgb.g) + ';' + std::to_string( (int) rgb.b) + 'm')

#define color_color(colr) colr.attr <<  color_fg_rgb(colr.fg) <<  color_bg_rgb(colr.bg)


#define attr_reset "\e[0m"
#define bold "\e[1m"
#define dim "\e[2m"
#define italic "\e[3m"
#define underline "\e[4m"
#define blink "\e[5m"
//idk what this means, docs said: "set inverse/reverse mode"
#define reverse "\e[7m"
#define hidden "\e[8m"
#define strike "\e[9m"


#define bold_reset "\e[22m"
#define dim_reset "\e[22m"
#define italic_reset "\e[23m"
#define underline_reset "\e[24m"
#define blink_reset "\e[25m"
//idk what this means, docs said: "set inverse/reverse mode"
#define reverse_reset "\e[27m"
#define hidden_reset "\e[28m"
#define strike_reset "\e[29m"
//seperate graphix modes with an semicolon (;) (presumeably to the current cursor location, untested)
#define set_mode_for_cell(mode) ("\e[1;34;"+mode+"m")

#define set_title_attr(title) ("\033]30;" + title + "\007")
#define set_title_static_attr(title) ("\033]30;" title "\007")

#define set_title_stream(title) "\033]30;" << title << "\007"
#define set_title(title) printf("\033]30;%s\007", title);
#define set_title_static(title) printf("\033]30;"#title"\007");


///Common Private Modes
/*
These are some examples of private modes, which are not defined by the specification, but are implemented in most terminals.
ESC Code Sequence 	Description
ESC[?25l 	        make cursor invisible
ESC[?25h 	        make cursor visible
ESC[?47l 	        restore screen
ESC[?47h 	        save screen
ESC[?1049h 	        enables the alternative buffer
ESC[?1049l 	        disables the alternative buffer
*/


#define cursor_invisible "\e[?25l"
#define cursor_visible "\e[?25h"
#define screen_save "\e[?47h"
#define screen_load "\e[?47l"
#define alt_buffer "\e[?1049h"
#define norm_buffer "\e[?1049l"

#define use_attr(attr) std::cout << attr;

//'\x41'
#define KEY_UP     0x00415b1b
//'\x42'
#define KEY_DOWN   0x00425b1b
//'\x43'
#define KEY_RIGHT  0x00435b1b
//'\x44'
#define KEY_LEFT   0x00445b1b






#define is_event(input_integer) (reinterpret_cast<u_short*>(&input_integer)[0] == EVENT_BASE)


#define EVENT_BASE 0x0000EEEE
#define RESIZE_EVENT (0x00010000 + EVENT_BASE)


#define CASE_STR(clause) \
case clause:\
    return #clause;

#define SET_X10_MOUSE               9
#define SET_VT200_MOUSE             1000
#define SET_VT200_HIGHLIGHT_MOUSE   1001
#define SET_BTN_EVENT_MOUSE         1002
#define SET_ANY_EVENT_MOUSE         1003

#define SET_FOCUS_EVENT_MOUSE       1004

#define SET_ALTERNATE_SCROLL        1007

#define SET_EXT_MODE_MOUSE          1005
#define SET_SGR_EXT_MODE_MOUSE      1006
#define SET_URXVT_EXT_MODE_MOUSE    1015
#define SET_PIXEL_POSITION_MOUSE    1016

#define USE_MOUSE SET_ANY_EVENT_MOUSE


#define enable_mouse(type) ("\e[?"+     std::to_string(type)    +"h")
#define disable_mouse(type) ("\e[?"+    std::to_string(type)    +"l")

#define is_mouse(inp) (reinterpret_cast<char*>(&inp)[0] == '\xFF')