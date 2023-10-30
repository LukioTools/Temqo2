#pragma once



#include <cstdlib>
#include <filesystem>
#include <string>
inline int system_ffmpeg(std::string i, std::string o){
    if(std::filesystem::exists(o)){
        return -1;
    }
    return system(("ffmpeg -nostats -loglevel panic -hide_banner -i " + i + " " + o).c_str());
}
