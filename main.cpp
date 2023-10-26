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
#include "lib/wm/mouse.hpp"


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



#define mv(x,y) std::cout << "\e["<< y+1 << ";"<< x+1 <<"H" << std::flush;
#define clear_all() std::cout << "\ec" << enable_mouse(USE_MOUSE) << std::flush;
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
//ESC[#G 	                moves cursor to column #
#define cursor_to_column(n) ("\e[" + std::to_string(n) + "G")

#define cursor_save "\e7"
#define cursor_load "\e8"
#define cursor_save_sco "\e[s"
#define cursor_load_sco "\e[u"

//use charachter width
#define color_fg(r,g,b) "\e[38;2;"<< r << ';'<< g << ';' << b << 'm'
#define color_bg(r,g,b) "\e[48;2;"<< r << ';'<< g << ';' << b << 'm'

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

#define use_attr(attr) std::cout << attr;

//'\x41'
#define KEY_UP     0x00415b1b
//'\x42'
#define KEY_DOWN   0x00425b1b
//'\x43'
#define KEY_RIGHT  0x00435b1b
//'\x44'
#define KEY_LEFT   0x00445b1b


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
inline std::string to_str(wm::MOUSE_BTN btn) noexcept{
    switch (btn) {
        CASE_STR(wm::M_UNDEFINED);
        CASE_STR(wm::M_NONE);
        CASE_STR(wm::M_RELEASE);
        CASE_STR(wm::M_LEFT);
        CASE_STR(wm::M_MIDDLE);
        CASE_STR(wm::M_RIGHT);
        CASE_STR(wm::M_SCRL_UP);
        CASE_STR(wm::M_SCRL_DOWN);
        CASE_STR(wm::M_LEFT_HILIGHT);
        CASE_STR(wm::M_MIDDLE_HILIGHT);
        CASE_STR(wm::M_RIGHT_HILIGHT);
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


#define is_event(input_integer) (reinterpret_cast<u_short*>(&input_integer)[0] == EVENT_BASE)



typedef const char* chtype;

std::string str_repeat(chtype c, int n) {
    std::ostringstream os;
    for(int i = 0; i < n; i++)
        os << c;
    return os.str();
}
//you can put t, b, l, r to nullptr so that the sides will not be printe
int box(wm::Window* window, chtype lt = "┌", chtype rt = "┐",chtype lb = "└", chtype rb = "┘", chtype t = "─", chtype b = "─", chtype r = "│", chtype l = "│"){
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

    if(t){
        std::string buffer;
        //print top
        buffer+=lt;
        buffer+=str_repeat(t, w-1); 
        buffer+=rt;
        mv(x, y);
        std::cout << buffer;
        buffer = "";
    }
    else{
        mv(x, y);
        std::cout << lt;
        mv(x+w, y);
        std::cout << rt;
    }
        
    if(b){
        std::string buffer;
        //print bottom
        buffer+=lb;
        buffer+=str_repeat(b, w-1);
        buffer+=rb;

        mv(x, y+h-1);
        std::cout << buffer;
    }
    else
    {
        mv(x, y+h-1);
        std::cout << lb;
        mv(x+w, y+h-1);
        std::cout << rb;
    }
    //M_LEFT and M_RIGHT
    for (size_t i = 1; i < h-1; i++)
    {
        mv(x, y+i);
        std::cout << l;
        mv(x+w, y+i);
        std::cout << r;
    }
    

    return 0;
}

#define ifnsp(var) var? var:" "
int box(wm::Space sp, chtype t = "─", chtype b = "─", chtype r = "│", chtype l = "│", chtype lt = "┌", chtype rt = "┐",chtype lb = "└", chtype rb = "┘"){
    if(!sp.exists()){
        return -1;
    }
    std::string buffer;
    
    {
        mv(sp.x, sp.y);
        buffer+=ifnsp(lt);
        for (size_t i = 0; i < sp.width()-2; i++)
            buffer+=ifnsp(t);
        buffer+=ifnsp(rt);
        std::cout << buffer;
    }
    buffer.clear();
    {
        mv(sp.x, sp.y+sp.h);
        buffer+=ifnsp(lb);
        for (size_t i = 0; i < sp.width()-2; i++)
            buffer+=ifnsp(b);
        buffer+=ifnsp(rb);
        std::cout << buffer;
    }

    //starts at one, since we wrote the top layer and is -1 because wrote bottom layer
    if(l || r){
        for (size_t i = 1; i < sp.height()-1; i++)
        {
            if(l){
                mv(sp.x, sp.y+i)
                std::cout << l;
            }
            if(r){
                mv(sp.x+sp.width(), sp.y+i)
                std::cout << r;
            }
        }
    }
    

    return 0;

}

#undef ifnsp

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
    x = std::cin.get()-'\x21';
    y = std::cin.get()-'\x21';
    ch = '\xFF' + (btn<<8) + (x << 16) + (y << 24);
            
    return ch;
}
enum SPLICE_TYPE{
    END_CUT,
    END_DOTS,
    BEGIN_CUT,
    BEGIN_DOTS,
};
typedef std::string(*wprintln_splicer)(wm::Window* window, std::string str, SPLICE_TYPE st);
/*prints a line whilst clipping exess*/
inline int wprintln(wm::Window* window, std::string str, SPLICE_TYPE st = SPLICE_TYPE::BEGIN_DOTS, wprintln_splicer csplicer = nullptr){
    
    if(!window)
        return -1;
    
    ///remove newlines, since they anyoing
    while (std::size_t idx = str.find_first_of('\n')) {
        if( idx == std::string::npos){
            break;
        }
        try{str.erase(idx);}
        catch(...){
            std::cout << "ioutof range";
            return -1;
        }
    }
    
    auto wspace = window->WriteableSpace();
    
    //trunctuate if it was larger than suposed to be
    if(str.length() > wspace.w){
        switch (st) {
            case BEGIN_DOTS:
                str = str.substr(str.length() - wspace.w, str.length());
                str.replace(0,3,"...");
                break;
            case END_DOTS:
                str = str.substr(0, wspace.w-3); //(-3 to make space for dots)
                str.append("...");
                break;
            case BEGIN_CUT:
                str = str.substr(str.length() - wspace.w, str.length());
                break;
            case END_CUT:
            default:
                if(csplicer){
                    str = csplicer(window, str, st);
                }
                if(str.length() > wspace.w){ //just in case cap that bitch
                    str = str.substr(0, wspace.w);
                }
                break;
        }
    }
    else{
        str.append(wspace.w-str.length(),' ');
    }

    std::cout << cursor_to_column(wspace.x) << str;

    return 0;
    /*
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
    */

}

/*just to make sure everythings inbounds*/
enum wprint_bmask : u_char{
    WP_B_CLIPPED = 0,
    WP_B_WRAPPED = 1,
    WP_B_REMOVED = 2,
};

inline int sprint(wm::Space wspace, std::string str){
    while (std::size_t idx = str.find_first_of('\n')) {
        if( idx == std::string::npos){
            break;
        }
        try{str.erase(idx);}
        catch(...){
            std::cout << "ioutof range";
            return -1;
        }
    };

    size_t iteration = 0;
    size_t pos = 0;
    std::basic_string<char> s;
    while(true){
        s.clear();

        s = str.substr(pos, wspace.width());
        pos+=wspace.width();


        std::cout<< cursor_to_column(wspace.x) << s << cursor_down(1); 
        iteration ++;
        if((iteration > wspace.h) || (s.length() < wspace.w)){
            break;
        }
    }
    return 0;
}

inline int wprint(wm::Window* window, std::string str){
    return sprint(window->WriteableSpace(), str);
};

int init(){
    
    //std::locale::global(std::locale("en_US.UTF-8"));

    use_attr(alt_buffer << enable_mouse(USE_MOUSE))
    signal(SIGWINCH, resize);
    tcgetattr(STDIN_FILENO, &oldt);
    clear_all();
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
std::string tst_str("");


const char* cursor = "^";


wm::Position mpos;


int main(int argc, char const *argv[])
{
    init();
    //display();
    auto space = new wm::Space(0,1,WIDTH,HEIGHT-1);
    auto w= new wm::Window(wm::ABSOLUTE, space, {1,1,2,2});
    //use_attr(cursor_invisible);

    std::string inp;
    while (true) {
        std::cout << set_title_static_attr("Hello World")    << std::flush;
        int ch = getch();
        clear_scr();
        if(ch == RESIZE_EVENT){
            space->refresh(0,1,WIDTH,HEIGHT-1);
        }

        mv(0,0)
        std::cout << *space;
        mv(WIDTH,HEIGHT)
        std::cout << "X";

        if(box(*space) == -1){
            std::cout << "box error";
        };
        std::cout.flush();


        

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
