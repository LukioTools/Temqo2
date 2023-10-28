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

#include "lib/wm/def.hpp"
#include "lib/wm/element.hpp"
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
    WIDTH = w.ws_col-1;
    HEIGHT = w.ws_row-1;
    resize_event = true;
}


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


#define ifnsp(var) var? var:" "
int box(wm::Space sp, chtype t = "─", chtype b = "─", chtype r = "│", chtype l = "│", chtype lt = "┌", chtype rt = "┐",chtype lb = "└", chtype rb = "┘"){
    if(!sp.exists()){
        return -1;
    }
    std::string buffer;

    auto s = sp.start();
    auto e = sp.end();
    
    
    for (size_t x = s.x; x <= e.x; x++)
    {   
        if(x == s.x)
            buffer+=lt;
        else if (x == e.x)
            buffer+=rt;
        else
            buffer+=t;
    }
    mv(s.x, s.y);
    std::cout << buffer;
    buffer.clear();

    for (size_t x = s.x; x <= e.x; x++)
    {   
        if(x == s.x)
            buffer+=lb;
        else if (x == e.x)
            buffer+=rb;
        else
            buffer+=b;
    }
    mv(s.x, e.y);
    std::cout << buffer;
    buffer.clear();


    for (size_t y = s.y; y <= e.y; y++)
    {
        if(y == s.y || y == e.y){
            continue;
        }
        mv(s.x, y)
        std::cout << l;
        mv(e.x, y)
        std::cout << r;
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
    
    auto wspace = window->wSpace();
    
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
    return sprint(window->wSpace(), str);
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

wm::Position mpos = {0,0};


bool enter = false;
int main(int argc, char const *argv[])
{
    init();
    //display();
    auto space = new wm::Space(0,1,WIDTH,HEIGHT-1);
    auto right_click_element_space = new wm::Space(0,0, 5, 7);

    auto rc_w = new wm::Element(right_click_element_space, {1,1,1,1});
    auto w= new wm::Element(space, {1,1,2,2});
    //use_attr(cursor_invisible);
    bool show_r_window = false;
    std::string inp;
    while (true) {
        int ch = getch();
        clear_scr();
        if(ch == 6939){
            break;
        }
        if(ch == RESIZE_EVENT){
            space->refresh(0,1,WIDTH,HEIGHT-2);
        }
        wm::MOUSE_BTN btn = wm::MOUSE_BTN::M_UNDEFINED;
        if(is_mouse(ch)){
            auto m = wm::parse_mouse(ch);
            mpos = m.pos;
            btn = m.btn;

            if(m.btn == wm::M_RIGHT){
                right_click_element_space->refresh(mpos.x, mpos.y, 15, 7);
                show_r_window = true;
            }
            if(!right_click_element_space->inside(mpos)){
                show_r_window = false;
            }


        }
        

        auto ws = w->wSpace();
        mv(0,0)
        printf("(Width: %i, Height: %i)", WIDTH, HEIGHT);
        std::cout << space->start() << space->end() << *space;
        mv(WIDTH,HEIGHT)

        //if(space->inside(mpos)){
        //    use_attr(color_bg(200,200,200) << color_fg(50,50,50))
        //    space->fill("0");
        //    use_attr(attr_reset);
        //}
        //else{
        //    space->fill("0");
        //}

        if(box(*space) == -1){
            std::cout << "box error";
        };
        
        if(ws.inside(mpos)){
            use_attr(color_bg(200,200,200) << color_fg(50,50,50))
            ws.fill("#");
            use_attr(attr_reset);
        }
        else{
            ws.fill("#");
        }

        {
            std::string str = "mouse: (x: " + std::to_string(mpos.x) + ", y: " + std::to_string(mpos.y) + ")";
            mv(WIDTH- str.length(), 0 );
            std::cout << str;
        }
        
        
        if(show_r_window == true){
            box(rc_w->aSpace());
            rc_w->wSpace().fill("#");
            auto ws = rc_w->wSpace();
        }
        
        
        use_attr(cursor_invisible);
        std::cout.flush();
        
        
    }
    delete w;
    delete space;
    deinit();
    return 0;
}
