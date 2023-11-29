#include "mpris_server.hpp"


int main(int argc, char const *argv[])
{
    //std::cout << "hello from dbus program\n";
    
    int i = 0;
    int64_t pos = 0;
    bool playing = false;

    mpris::Server server("genericplayer");

    server.set_identity("A generic player");
    server.set_supported_uri_schemes({ "file" });
    server.set_supported_mime_types({ "application/octet-stream", "text/plain" });
    server.set_metadata({
        { mpris::Field::TrackId,    "/1"             },
        { mpris::Field::Album,      "an album"       },
        { mpris::Field::Title,      "best song ever" },
        { mpris::Field::Artist,     "idk"            },
        { mpris::Field::Length,     1000             },
        { mpris::Field::ArtUrl, "file:///home/pikku/code/tqo2/gluttony.png"},
    });
    server.set_maximum_rate(2.0);
    server.set_minimum_rate(0.1);

    server.on_quit([&] { std::exit(0); });
    server.on_next([&] { i++; 
        std::cout << "next" << std::endl;
    });
    server.on_previous([&] { i--; std::cout << "prev" << std::endl;});
    server.on_pause([&] {
        playing = false;
        server.set_playback_status(mpris::PlaybackStatus::Paused);
        std::cout << "playing: " << (playing ? "true":"false" ) << std::endl;
    });
    server.on_play_pause([&] {
        playing = !playing;
        server.set_playback_status(playing ? mpris::PlaybackStatus::Playing
                                           : mpris::PlaybackStatus::Paused);
        std::cout << "playing: " << (playing ? "true":"false" ) << std::endl;
    });
    server.on_stop([&] {
        playing = false;
        server.set_playback_status(mpris::PlaybackStatus::Stopped);
        std::cout << "playing: " << (playing ? "true":"false" ) << std::endl;
    });
    server.on_play([&] {
        playing = true;
        server.set_playback_status(mpris::PlaybackStatus::Playing);
        std::cout << "playing: " << (playing ? "true":"false" ) << std::endl;
    });
    server.on_seek(        [&] (int64_t p) { pos += p; server.set_position(pos); std::cout << "position: " << pos << std::endl;});
    server.on_set_position([&] (int64_t p) { pos  = p; server.set_position(pos); std::cout << "position: " << pos << std::endl;});
    server.on_open_uri([&] (std::string_view uri) { printf("not opening uri, sorry\n"); });

    server.on_loop_status_changed([&] (mpris::LoopStatus status) {
        std::cout << "loop: ";
        switch (status)
        {
        case (mpris::LoopStatus::None):
            std::cout << "None";
            break;
        case (mpris::LoopStatus::Playlist):
            std::cout << "Playlist";
            break;
        case (mpris::LoopStatus::Track):
            std::cout << "Track";
            break;
        default:
            break;
        }
        std::cout << std::endl;

    });
    server.on_rate_changed([&] (double rate) {
        std::cout << "vol: " << rate << '%' << std::endl;
    });
    server.on_shuffle_changed([&] (bool shuffle) {
        std::cout << "shuffle: " << (shuffle ? "true":"false" ) <<std::endl;
    });
    server.on_volume_changed([&] (double vol) {
        std::cout << "vol: " << vol << '%' << std::endl;
    });
    


    server.start_loop_async();
    

    for (;;) {
        if (playing) {
            printf("%ld\n", pos);
            pos++;
            server.set_position(pos);
        } else {
            printf("i'm paused (or stopped)\n");
        }
        sleep(1);
    }

    return 0;


    
    return 0;
}
