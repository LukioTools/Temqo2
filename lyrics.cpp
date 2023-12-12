#include "custom/lyrics.hpp"


int main(int argc, char const *argv[])
{
    //so try to parse lyrics
    std::string song_path = "/home/pikku/Music/Rap/Eminem - Beautiful (Official Music Video)-lgT1AidzRWM.mp3";
    auto match = scan_lyrics(song_path);
    if(!match.empty()){
        std::cout << "Found: " << match << std::endl;
        std::vector<Lyric> lv;
        parse_lyrics(match, lv);
        for (auto& l : lv) {
            std::cout << l.end_time << ' ' << l.str << std::endl;
        }
    }

    return 0;
}
