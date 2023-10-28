
//#include "lib/audio/audio_backend.hpp"
#include "lib/audio/audio_backend.hpp"
#include "lib/audio/playlist.hpp"
#include <iostream>
#include <vector>

int main(int argc, char const *argv[])
{
    audio::Playlist p;

    p.add(".");

    printf("Found:\n");
    for (auto e : p) {
        std::cout << e << std::endl;
    }



    audio::init(p[0].c_str());

    audio::play();
    printf("\rPlaying: %s", p[0].c_str());

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
        else if(c == 'w'){
            audio::play();
        }
        else if(c == 's'){
            audio::stop();
        }
        else if(c == 'c'){
            audio::playing ? audio::stop() : audio::play();
        }
        else if(c == '+'){
            audio::vol(1.1);
            printf("Volume %f\n", audio::volume.load());
        }
        else if(c == '-'){
            audio::vol(0.9);
            printf("Volume %f\n", audio::volume.load());
        }
        else if(c == 'n'){
            auto s = p.next();
            audio::load_next(s.c_str(), true);
            printf("\rPlaying: %s", s.c_str());
        }
    }
    

    audio::stop();

    audio::deinit();

    return 0;
}
