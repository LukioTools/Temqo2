#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <mutex>
#include <condition_variable> // std::condition_variable
#include <thread>
#include <sys/ioctl.h>
#include <sstream>

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
#define clear() std::cout << "\ec" << enable_mouse(USE_MOUSE) << std::flush;
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

#define set_title_attr(title) ("\033]2;" + title + "\007")
#define set_title_static_attr(title) ("\033]2;" title "\007")

#define set_title(title) printf("\033]0;%s\007", title);
#define set_title_static(title) printf("\033]0;"#title"\007");


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
    M_UNDEFINED = 0,
    M_NONE = '\x42',
    M_RELEASE = '\x22',
    M_LEFT = '\x1f',
    M_MIDDLE = '\x20',
    M_RIGHT = '\x21',
    M_SCRL_UP = '\x5F',
    M_SCRL_DOWN = '\x60',
    M_LEFT_HILIGHT = '\x3f',
    M_MIDDLE_HILIGHT = '\x40',
    M_RIGHT_HILIGHT = '\x41',
};

enum KEY : u_char{
    K_UNDEFINED = 0,
    K_UP, 
    K_DOWN, 
    K_LEFT,
    K_RIGHT, 
};

#define CASE_STR(clause) \
case clause:\
    return #clause;
inline std::string to_str(MOUSE_BTN btn) noexcept{
    switch (btn) {
        CASE_STR(M_UNDEFINED);
        CASE_STR(M_NONE);
        CASE_STR(M_RELEASE);
        CASE_STR(M_LEFT);
        CASE_STR(M_MIDDLE);
        CASE_STR(M_RIGHT);
        CASE_STR(M_SCRL_UP);
        CASE_STR(M_SCRL_DOWN);
        CASE_STR(M_LEFT_HILIGHT);
        CASE_STR(M_MIDDLE_HILIGHT);
        CASE_STR(M_RIGHT_HILIGHT);
        default:
            return "UNKNOWN";
    }
}
inline std::string to_str(KEY key){
    switch (key)
    {
    CASE_STR(K_UNDEFINED);
    CASE_STR(K_UP);
    CASE_STR(K_DOWN);
    CASE_STR(K_LEFT);
    CASE_STR(K_RIGHT);
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
    u_short x = 0;
    u_short y = 0;
    MOUSE_BTN btn = MOUSE_BTN::M_UNDEFINED;
    u_char valid = 1;
};


#define is_event(input_integer) (reinterpret_cast<u_short*>(&input_integer)[0] == EVENT_BASE)


#define is_mouse(inp) (reinterpret_cast<char*>(&inp)[0] == '\xFF')

typedef const char* chtype;

std::string str_repeat(chtype c, int n) {
    std::ostringstream os;
    for(int i = 0; i < n; i++)
        os << c;
    return os.str();
}

int box(wm::Window* window, chtype rt = "┌", chtype lt = "┐",chtype rb = "└", chtype lb = "┘", chtype t = "─", chtype b = "─", chtype r = "│", chtype l = "│"){
    if(!window){
        return -1;
    }
    wm::Space wspace = window->AbsoluteSpace();
    if(!wspace.exists()){
        mv(2, 2);
        std::cout << wspace;
        return -3;
    }
    auto x = window->space->x;
    auto y = window->space->y;
    auto w = window->space->w;
    auto h = window->space->h;

    if(!x || !y || !w || !h){
        return -2;
    }
    if(w < 2 || h < 3){
        return 1;
    }
    std::string buffer;

    //print top
    {
        buffer+=rt;
        buffer+=str_repeat(t, w-2);
        buffer+=lt;

        mv(x, y);
        std::cout << buffer;
    }
    //print bottom
    buffer = "";
    {
        buffer+=rb;
        buffer+=str_repeat(b, w-2);
        buffer+=lb;

        mv(x, y+h-1);
        std::cout << buffer;
    }
    //M_LEFT and M_RIGHT
    buffer = l;
    std::string M_RIGHT = r;
    for (size_t i = 1; i < h-1; i++)
    {
        mv(x, y+i);
        std::cout << buffer;
        mv(x+w-1, y+i);
        std::cout << M_RIGHT;
    }
    

    return 0;
}



KEY is_key(int input) noexcept{
    auto ptr = reinterpret_cast<char*>(&input);
    if(ptr[0] != '\x1b' || ptr[1] != '\x5b'|| ptr[2] > '\x44' || ptr[2] < '\x41'){
        return K_UNDEFINED;
    }
    switch (ptr[2])
    {
    case '\x41':    return K_UP;
    case '\x42':    return K_DOWN;
    case '\x43':    return K_RIGHT;
    case '\x44':    return K_LEFT;
    default:        return K_UNDEFINED;
    }
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

    if(c !='\x5b'){ //cursor 0x004x5b1b x = 1-4 (up, down, M_LEFT, M_RIGHT)
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
    
    //std::locale::global(std::locale("en_US.UTF-8"));

    use_attr(alt_buffer << enable_mouse(USE_MOUSE))
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
    use_attr(disable_mouse(USE_MOUSE) << norm_buffer << cursor_visible);
    return 0;
}

void display(){
    clear();
    mv(3,3);

    std::string tst_str("ABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAO");
    printf("Hello\nWorld width:%u, len:%zu", WIDTH, tst_str.length());
    std::cout << std::flush;
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mv(3,5);
    wprintln(nullptr, tst_str, SPLICE_TYPE::END_CUT);
}
const char* cursor = "^";
int main(int argc, char const *argv[])
{
    init();
    //display();
    auto space = new wm::Space(1,2,WIDTH,HEIGHT-2);
    auto w= new wm::Window(wm::ABSOLUTE, space, {1,1,2,2});
    //use_attr(cursor_invisible);


    while (true) {
        std::cout << set_title_static_attr("Hello World")    << std::flush;
        int ch = getch();
        clear_scr();
        if(ch == RESIZE_EVENT){
        //    display();
            space->refresh(2, 2, WIDTH-3,HEIGHT-2);
        }
        /*
        mv(3,2);
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

        mv(3,6);
        */ 

        
        box(w);
        //auto spc = w->WriteableSpace();

        use_attr(cursor_home);
        if(is_mouse(ch)){
            auto mouse_input = parse_mouse(ch);
            std::cout << "mx:"<< (int) mouse_input.x << " my: "<< (int) mouse_input.y << std::flush;
            mv((int) mouse_input.x, (int) mouse_input.y);
            //std::cout << cursor;
        }
        else if(KEY key = is_key(ch)){
            std::cout<< "key: " << (int) key << to_str(key);
        }
        else{
            mv(1,1);
            std::cout<< "char: " << (char) ch;
        }
        
        mv(1, HEIGHT)
        std::cout << WIDTH <<":"<<HEIGHT;

        //mv(0,0);
        if(ch == 6939){
            break;
        }
    }
    delete w;
    delete space;
    deinit();
    return 0;
}
