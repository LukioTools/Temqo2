#include <atomic>
#include <cstdio>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <mutex>
#include <condition_variable> // std::condition_variable
#include <thread>
#include <sys/ioctl.h>

#include "lib/wm/row.hpp"

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

struct termios oldt, newt;
unsigned int WIDTH = 0, HEIGHT = 0;
#define EVENT_BASE 0x0000EEEE
#define RESIZE_EVENT (0x00010000 + EVENT_BASE)
std::atomic_bool resize_event;

void resize(int sig){
    if(sig != SIGWINCH) {return;}
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH = w.ws_col;
    HEIGHT = w.ws_row;
    resize_event = true;
}

#define enable_mouse(type) ("\e[?"+     std::to_string(type)    +"h")
#define disable_mouse(type) ("\e[?"+    std::to_string(type)    +"l")


#define mv(x,y) std::cout << "\e["<< y << ";"<< x <<"H" << std::flush;
#define clear() std::cout << "\ec" << enable_mouse(USE_MOUSE)<< std::flush;
#define clear_scr() std::cout << "\e[2J" << std::flush;
#define clear_row() std::cout << "\e[2K" << std::flush;
#define clear_curs_eol() std::cout << "\e[0K" << std::flush;
#define clear_curs_sol() std::cout << "\e[1K" << std::flush;

#define alert '\a'

#define cursor_home "\e[H"
#define cursor_up(n)    ("\e["+std::to_string(n)+"A")
#define cursor_down(n)  ("\e["+std::to_string(n)+"B")
#define cursor_right(n) ("\e["+std::to_string(n)+"C")
#define cursor_left(n)  ("\e["+std::to_string(n)+"D")
#define cursor_up_scrl "\eM"
#define cursor_save "\e7"
#define cursor_load "\e8"
#define cursor_save_sco "\e[s"
#define cursor_load_sco "\e[u"

//use charachter width
#define color_fg(r,g,b) std::cout << "\e[38;2;"<< r << ';'<< g << ';' << b << 'm'
#define color_bg(r,g,b) std::cout << "\e[48;2;"<< r << ';'<< g << ';' << b << 'm'

#define color_fg_str(r,g,b) ("\e[38;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm')
#define color_bg_str(r,g,b) ("\e[48;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm')


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

#define use_attr(attr) std::cout << attr << std::flush;

//'\x41'
#define KEY_UP     0x00415b1b
//'\x42'
#define KEY_DOWN   0x00425b1b
//'\x43'
#define KEY_RIGHT  0x00435b1b
//'\x44'
#define KEY_LEFT   0x00445b1b

///!!NOT GUARANTEED!!///
enum MOUSE_BTN : unsigned char{
    UNDEFINED = 0,
    NONE = '\x42',
    RELEASE = '\x22',
    LEFT = '\x1f',
    MIDDLE = '\x20',
    RIGHT = '\x21',
    SCRL_UP = '\x5F',
    SCRL_DOWN = '\x60',
    LEFT_HILIGHT = '\x3f',
    MIDDLE_HILIGHT = '\x40',
    RIGHT_HILIGHT = '\x41',
};

#define CASE_STR(clause) \
case clause:\
    return #clause;
inline std::string to_str(MOUSE_BTN btn) noexcept{
    switch (btn) {
        CASE_STR(UNDEFINED);
        CASE_STR(NONE);
        CASE_STR(RELEASE);
        CASE_STR(LEFT);
        CASE_STR(MIDDLE);
        CASE_STR(RIGHT);
        CASE_STR(SCRL_UP);
        CASE_STR(SCRL_DOWN);
        CASE_STR(LEFT_HILIGHT);
        CASE_STR(MIDDLE_HILIGHT);
        CASE_STR(RIGHT_HILIGHT);
        default:
            return "UNKNOWN";
    }
}
typedef int event; 
inline std::string to_str_event(event inp){
    switch (inp) {
        case RESIZE_EVENT:
            return "RESIZE";
        default:
            return "UNKNOWN_EVENT";
    }
}

struct MOUSE_INPUT {
    u_char x = 0;
    u_char y = 0;
    MOUSE_BTN btn = MOUSE_BTN::UNDEFINED;
    u_char valid = 1;
};


#define is_event(input_integer) (reinterpret_cast<u_short*>(&input_integer)[0] == EVENT_BASE)


#define is_mouse(inp) (reinterpret_cast<char*>(&inp)[0] == '\xFF')

bool is_key(int input) noexcept{
    auto ptr = reinterpret_cast<char*>(&input);
    if(ptr[0] != '\x1b' || ptr[1] != '\x5b' || ptr[3] > '\x44' || ptr[3] < '\x41'){
        return false;
    }
    return true;
}



MOUSE_INPUT parse_mouse(int input) noexcept{
    auto ptr = reinterpret_cast<unsigned char*>(&input);
    char checksum = static_cast<char>(ptr[0]);
    MOUSE_BTN btn = static_cast<MOUSE_BTN>(ptr[1]);
    u_char x = ptr[2];
    u_char y = ptr[3];
    return {x,y,btn,checksum == '\xFF'};
}

int getch(){
    if(resize_event){
        resize_event = false;
        return RESIZE_EVENT;
    }

    int ch = std::cin.get();
    if(ch != '\x1b'){//escape
        return ch;
    }

    int c = std::cin.get();
    ch+=c<<8;

    if(c !='\x5b'){ //cursor 0x004x5b1b x = 1-4 (up, down, left, right)
        return ch;
    }

    c = std::cin.get();
    
    if(c != '\x4d'){//mouse event
        ch+=c<<16;
        return ch;
    }

    unsigned int btn = 0, x = 0, y = 0;
    btn = std::cin.get();
    x = std::cin.get()-'\x20';
    y = std::cin.get()-'\x20';
    ch = '\xFF' + (btn<<8) + (x << 16) + (y << 24);
            
    return ch;
}
enum SPLICE_TYPE{
    END_CUT,
    END_DOTS,
    BEGIN_CUT,
    BEGIN_DOTS,
};

/*prints a line whilst clipping exess*/
inline void wprintln(wm::Window* window = nullptr, std::string str = "", SPLICE_TYPE st = SPLICE_TYPE::BEGIN_DOTS){
    if(window == nullptr){
        while (auto idx = str.find_first_of('\n') != std::string::npos) {
            str.erase(idx);
        }
        if(str.length() > WIDTH){
            switch (st) {
                case BEGIN_DOTS:
                    str = str.substr(str.length() - WIDTH, str.length());
                    str.replace(0,3,"...");
                    break;
                case END_DOTS:
                    str = str.substr(0, WIDTH-3); //(-3 to make space for dots)
                    str.append("...");
                    break;
                case BEGIN_CUT:
                    str = str.substr(str.length() - WIDTH, str.length());
                    break;
                case END_CUT:
                default:
                    str = str.substr(0, WIDTH);
                    break;
            }
        }
        std::cout << str << std::flush;
    }
}

int init(){
    use_attr(alt_buffer << enable_mouse(SET_X10_MOUSE))
    signal(SIGWINCH, resize);
    tcgetattr(STDIN_FILENO, &oldt);
    clear();
    resize(SIGWINCH);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    return 0;
}

int deinit(){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    use_attr(disable_mouse(SET_X10_MOUSE) << norm_buffer);
    return 0;
}

void display(){
    clear();
    mv(1,2);

    std::string tst_str("ABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAO");
    printf("Hello\nWorld width:%u, len:%zu", WIDTH, tst_str.length());
    std::cout << std::flush;
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mv(1,4);
    wprintln(nullptr, tst_str, SPLICE_TYPE::END_CUT);
}

int main(int argc, char const *argv[])
{
    init();
    display();


    while (true) {
        int ch = getch();
        if(ch == RESIZE_EVENT){
            display();
        }
        mv(1,1);
        clear_row();

        std::string custom = "";
        if(is_mouse(ch)){
            auto mouse = parse_mouse(ch);
            custom = "mouse: (x: " + std::to_string(mouse.x) + " y: " + std::to_string(mouse.y) + " btn: "  + to_str(mouse.btn) + ") ";
        }
        else if( is_key(ch)){
            custom = "key: " + std::to_string(ch);
        }
        else if( is_event(ch)){
            
            custom = "event: " + to_str_event(static_cast<event>(ch));
        }
        else {
            custom = "idk";
        }
        
        printf("%schar(%i)(0x%08x)(%s):%s %c", color_fg_str(50,255,50).c_str(), ch,ch, custom.c_str(),attr_reset, ch);
        
        std::cout << std::flush;
        

        //mv(0,0);
        if(ch == 6939){
            break;
        }
    }


    deinit();
    return 0;
}
