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

struct termios oldt, newt;
unsigned int WIDTH = 0, HEIGHT = 0;


void resize(int sig){
    if(sig != SIGWINCH) {return;}
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH = w.ws_col;
    HEIGHT = w.ws_row;
}

#define mv(x,y) std::cout << "\e["<< y << ";"<< x <<"H" << std::flush;
#define clear() std::cout << "\ec" << std::flush;
#define clear_row() std::cout << "\e[2K" << std::flush;
#define clear_curs_eol() std::cout << "\e[0K" << std::flush;



//use charachter width
#define color_fg(r,g,b) std::cout << "\e[38;2;"<< r << ';'<< g << ';' << b << 'm'
#define color_bg(r,g,b) std::cout << "\e[48;2;"<< r << ';'<< g << ';' << b << 'm'

#define color_fg_str(r,g,b) ("\e[38;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm')
#define color_bg_str(r,g,b) ("\e[48;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + 'm')

#define attr_reset "\e[0m"

#define enable_mouse(type) ("\e[?"+std::to_string(type)+"h")
#define disable_mouse(type) ("\e[?"+std::to_string(type)+"l")


//'\x41'
#define KEY_UP     0x00415b1b
//'\x42'
#define KEY_DOWN   0x00425b1b
//'\x43'
#define KEY_RIGHT  0x00435b1b
//'\x44'
#define KEY_LEFT   0x00445b1b


enum MOUSE_BTN : unsigned char{
    UNDEFINED = 0,
    LEFT = '\x1f',
    MIDDLE = '\x20',
    RIGHT = '\x21',
};

inline std::string to_str(MOUSE_BTN btn) noexcept{
    switch (btn) {
        case LEFT:
            return "LEFT";
        case MIDDLE:
            return "MIDDLE";
        case RIGHT:
            return "RIGHT";
        default:
            return "UNDEFINED";
    }
}

struct MOUSE_INPUT {
    u_char x = 0;
    u_char y = 0;
    MOUSE_BTN btn = MOUSE_BTN::UNDEFINED;
    u_char valid = 1;
};



#define is_mouse(inp) (reinterpret_cast<char*>(&inp)[0] == '\xFF')

bool is_key(int input) noexcept{
    auto ptr = reinterpret_cast<char*>(&input);
    if(ptr[0] != '\x1b' || ptr[1] != '\x5b' || ptr[3] > '\x44' || ptr[3] < '\41'){
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
    BEGIN_CUT,
    END_CUT,
    END_DOTS,
    BEGIN_DOTS,
};

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
    signal(SIGWINCH, resize);
    tcgetattr(STDIN_FILENO, &oldt);
    std::cout << enable_mouse(SET_X10_MOUSE) << std::flush;
    clear();
    resize(SIGWINCH);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    return 0;
}

int deinit(){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << disable_mouse(SET_X10_MOUSE) << std::flush;
    return 0;
}



int main(int argc, char const *argv[])
{
    init();
    while (true) {
        int ch = getch();
        mv(1,1);
        clear();

        std::string custom = "";
        if(is_mouse(ch)){
            auto mouse = parse_mouse(ch);
            custom = "mouse: (x: " + std::to_string(mouse.x) + " y: " + std::to_string(mouse.y) + " btn: "  + to_str(mouse.btn) + ") ";
        }
        else if( is_key(ch)){
            custom = "key: ";
        }
        
        
        printf("%schar(%i)(0x%08x)(%s):%s %c", color_fg_str(50,255,50).c_str(), ch,ch, custom.c_str(),attr_reset, ch);
        
        std::cout << std::flush;
        mv(1,2);

        std::string tst_str("ABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAOABCDEFGHIJKLMNOPQRSTUVWXYZOAO");
        printf("Hello\nWorld width:%u, len:%zu", WIDTH, tst_str.length());

        std::cout << std::flush;
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        mv(1,4);
        wprintln(nullptr, tst_str, 
        SPLICE_TYPE::END_CUT);

        //mv(0,0);
        if(ch == 6939){
            break;
        }
    }


    deinit();
    return 0;
}
