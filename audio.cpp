
#include "lib/audio/audio_backend.hpp"
#include <cstdio>

int main(int argc, char const *argv[])
{
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
    }
    

    audio::stop();

    audio::deinit();

    return 0;
}
