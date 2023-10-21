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
int WIDTH = -1, HEIGHT = -1;


void resize(int sig){
    if(sig != SIGWINCH) {return;}
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH = w.ws_col;
    HEIGHT = w.ws_row;
}

#define mv(x,y) std::cout << "\e["<< x << ";"<< y<<"H" << std::flush;
#define clear() std::cout << "\e[2J" << std::flush;
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

enum MOUSE_BTN : unsigned char{
    UNDEFINED = 0,
    LEFT = '\x1f',
    MIDDLE = '\x20',
    RIGHT = '\x21',
};


struct MOUSE_INPUT {
    u_char x = 0;
    u_char y = 0;
    MOUSE_BTN btn = MOUSE_BTN::UNDEFINED;
    u_char valid = 1;

    
};
std::string mouse_btn_to_string(MOUSE_BTN btn){
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

MOUSE_INPUT parse_mouse(int input){
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
        mv(0,0);
        clear_row();
        
        auto mouse = parse_mouse(ch);
        printf("%schar(%i)(0x%08x)(mouse: %s):%s %c", color_fg_str(50,255,50).c_str(), ch,ch, mouse.valid ? ("x: " + std::to_string(mouse.x) + " y: " + std::to_string(mouse.y) + " btn: " + mouse_btn_to_string(mouse.btn)).c_str() : "false",attr_reset, ch);
        std::cout << std::flush;
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));

        //mv(0,0);
        if(ch == 6939){
            break;
        }
    }


    deinit();
    return 0;
}
