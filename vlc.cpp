#include <cstdio>
#include <iostream>
#include "lib/audio/vlc.hpp"


void on_end(){
    std::cout << "Current song ended\n";
}
void on_time(libvlc_time_t time){
    std::cout << "Time: " << audio_vlc::played::get_s().count() << "/" << audio_vlc::lenght::get_s().count() <<"\n";
}

int main() {
    if(audio_vlc::init()){
        std::cout << "audio_vlc::init() failed\n";
        exit(1);
    };

    audio_vlc::on::end(on_end);
    audio_vlc::on::time(on_time);
    audio_vlc::load("irridecent.mp3");
    audio_vlc::play();

    getchar();
    audio_vlc::stop();
    audio_vlc::load("stardust.mp3");
    audio_vlc::play();
    getchar();

    std::cout << "Exxitttih\n";


    audio_vlc::stop();


    return 0;
}
