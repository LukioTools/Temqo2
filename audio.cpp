
//#include "lib/audio/audio_backend.hpp"
#include "lib/audio/audio_backend.hpp"
#include "lib/audio/playlist.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include "lib/path/filename.hpp"
#include "lib/path/absolute.hpp"
#include <termios.h>
struct termios oldt, newt;

audio::Playlist p;


void cb(){
    auto s = p.next();
    auto filename = path::filename(s);
    audio::load_next(s.c_str(), true);
    printf("Playing: %s\n\e]30;%s\a", s.c_str(), filename.c_str());
}

int init(){
    
    //std::locale::global(std::locale("en_US.UTF-8"));

    tcgetattr(STDIN_FILENO, &oldt);
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
    std::string pth = "~/Music/";
    std::cout << pth << std::endl;
    pth = path::waveline(pth);
    std::cout << pth << std::endl;

    p.load("playlist.pls");
    //p.add(pth, true);
    //p.save("playlist.pls");

    printf("Found:\n");
    for (auto e : p) {
        std::cout << e << std::endl;
    }
    if(p.files.size() == 0){
        std::cout << "No files found! \n";
        return -1;
    }

    audio::songEndedCallback = cb;
    audio::init(p[0].c_str());

    audio::play();
    printf("\rPlaying: %s", p[0].c_str());

    init();
    while (true) {
        char c = getchar();
        switch (c)
        {
        case 'q':{
            goto exit;
            break;
        }
        case 'd':{
            audio::seek(std::chrono::seconds(5));
            break;
        }
        case 'a':{
            audio::seek(std::chrono::seconds(-5));
            break;
        }
        case 's':{
            p.shuffle();
            printf("Suffling list...\n");
            //no break so it executes the next thing
        }
        case 'n':{
            auto s = p.next();
            auto filename = path::filename(s);
            audio::load_next(s.c_str(), true);
            printf("Playing: %s\n\e]30;%s\a", s.c_str(), filename.c_str());
            break;
        }
        case 'b':{
            auto s = p.prev();
            auto filename = path::filename(s);
            audio::load_next(s.c_str(), true);
            printf("Playing: %s\n\e]30;%s\a", s.c_str(), filename.c_str());
            break;
        }
        case 'c':{
            audio::playing ? audio::stop() : audio::play();
            break;
        }
        case '+':{
            audio::vol(1.1);
            printf("Volume %f\n", audio::volume.load());
            break;
        }
        case '-':{
            audio::vol(0.9);
            printf("Volume %f\n", audio::volume.load());
            break;
        }
        
        }
    }
    exit:
    audio::stop();

    audio::deinit();
    deinit();

    return 0;
}
