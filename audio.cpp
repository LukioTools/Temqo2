
//#include "lib/audio/audio_backend.hpp"
#include "lib/audio/scandir.hpp"
#include <iostream>
#include <vector>

int main(int argc, char const *argv[])
{
    std::vector<std::string> vec;
    audio::scan(vec, "./", true);


    for (auto e : vec) {
        std::cout << e << std::endl;
    }

    /*
    const char* next = "irridecent.mp3";
    audio::init("stardust.mp3");

    audio::play();

    while (true) {
        char c = getchar();

        if(c == 'q'){
            break;
        }
        else if(c == 'd'){
            audio::seek(std::chrono::seconds(5));
        }
        else if(c == 'a'){
            audio::seek(std::chrono::seconds(-5));
        }
        else if(c == 'n'){
            audio::load_next(next, true);
        }
    }
    

    audio::stop();

    audio::deinit();
    */

    return 0;
}
