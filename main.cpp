#include <atomic>
#include <cstdio>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <mutex>
#include <condition_variable> // std::condition_variable
#include <thread>
#include <sys/ioctl.h>

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

int getch(){
    int ch = std::cin.get();
    if(ch == 27){//escape
        int c = std::cin.get();
        ch+=c<<8;
        if(c=='\x5b'){ //cursor 0x004x5b1b x = 1-4 (up, down, left, right)
            c = std::cin.get();
            ch+=c<<16;
        }
    }
    return ch;
}

int init(){
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
    return 0;
}



int main(int argc, char const *argv[])
{
    init();
    while (true) {
        int ch = getch();
        mv(0,0);
        clear_row();
        printf("char(%i)(0x%08x): %c", ch,ch,ch);
        //mv(0,0);
        if(ch == 6939){
            break;
        }
    }


    deinit();
    return 0;
}
