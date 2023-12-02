

#include <iostream>
#include <string>
#include "lib/audio/extract_img.hpp"


int main(int argc, char const *argv[])
{
    

    std::string filename = "/home/pikku/Music/MyMisx/Kaupunkilaiskanoja ja maalaisnautoja-lEC4UF983KE.mp3";

    auto metadata = audio::extra::getMetadata(filename);

    if(!metadata.has_value())
        return 1;

    std::cout << "title: " << metadata->title << std::endl;
    std::cout << "album: " << metadata->album.toCString(true) << std::endl;
    std::cout << "artist: " << metadata->artist << std::endl;
    std::cout << "genre: " << metadata->genre << std::endl;
    std::cout << "track: " << metadata->track << std::endl;
    std::cout << "year: " << metadata->year << std::endl;
    std::cout << "comment: " << metadata->comment << std::endl;


    return 0;
}
