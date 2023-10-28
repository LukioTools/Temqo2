
//#include "lib/audio/audio_backend.hpp"
#include "lib/audio/audio_backend.hpp"
#include "lib/audio/playlist.hpp"
#include <iostream>
#include <thread>
#include <vector>

audio::Playlist p;


void cb(){
    auto s = p.next();
    audio::load_next(s.c_str(), true);
    printf("Playing: %s\n", s.c_str());
}


int main(int argc, char const *argv[])
{


    p.add("/home/pikku/Music/", true);

    printf("Found:\n");
    for (auto e : p) {
        std::cout << e << std::endl;
    }

    audio::songEndedCallback = cb;
    audio::init(p[0].c_str());

    audio::play();
    printf("\rPlaying: %s", p[0].c_str());

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
        }
        case 'n':{
            auto s = p.next();
            audio::load_next(s.c_str(), true);
            printf("\rPlaying: %s", s.c_str());
            break;
        }
        case 'b':{
            auto s = p.prev();
            audio::load_next(s.c_str(), true);
            printf("\rPlaying: %s", s.c_str());
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

    return 0;
}
