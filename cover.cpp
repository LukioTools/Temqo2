#include "lib/audio/extract_img.hpp"
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int WIDTH = w.ws_col;
    int HEIGHT = w.ws_row;

    
    auto str = audio::extra::get("irridecent.mp3", WIDTH, HEIGHT);
    std::cout << str << std::endl;
    std::cout << str.length() << std::endl;
    return 0;
}
