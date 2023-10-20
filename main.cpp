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


int getch(){
    return std::cin.get();
}

int init(){
    signal(SIGWINCH, resize);
    tcgetattr(STDIN_FILENO, &oldt);
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
    int ch = getch();
    printf("char(%i): %c", ch,ch);


    deinit();
    return 0;
}
