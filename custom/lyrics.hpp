#pragma once

#include "../lib/audio/scandir.hpp"
#include "../lib/path/filename.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

struct Lyric
{   //in milliseconds
    size_t end_time = 0;
    std::string str;
};


inline std::string scan_lyrics(const std::string& song_path){
    std::string song_name =  path::pathfilebasename(song_path);
    std::string dir =  song_path.substr(0, song_path.find_last_of('/')) + '/';
    std::string match;

    audio::scanfncfnthr(dir, false, [&](const std::string& str){
        if(path::pathfilebasename(str) == song_name && path::fileext(str) == ".vtt")
                return true;
        return false;
    }, [&](std::string str){
        match = str;
    });
    return match;
}

inline void scan_all_lyrics(std::vector<std::string>& lyric_files, const std::string& song_path){
    std::string song_name =  path::pathfilebasename(song_path);
    std::string dir =  song_path.substr(0, song_path.find_last_of('/')) + '/';
    audio::scanfncfnthr(dir, false, [&](const std::string& str){
        if(path::pathfilebasename(str) == song_name && path::fileext(str) == ".vtt")
                return true;
        return false;
    }, [&](std::string str){
        lyric_files.push_back(str);
    });
}


inline std::regex is_timestamp("^\\d{2}:\\d{2}:\\d{2}\\.\\d{3} --> \\d{2}:\\d{2}:\\d{2}\\.\\d{3}.*$");
inline size_t parse_timestamp(const std::string& line){
    size_t out = 0; 
    auto first = line.find_first_of(':');
    std::string hours = line.substr(0, first);
    out+=std::stoi(hours)*60*60*1000;
    first++;
    auto second = line.find(':', first);
    std::string mins = line.substr(first, second-first);
    out+=std::stoi(mins)*60*1000;
    second++;
    auto third = line.find('.', second);
    std::string seconds = line.substr(second, third-second);
    out+=std::stoi(seconds)*1000;
    third++;
    auto fourth = line.find(' ', third);
    std::string msec = line.substr(third, fourth-third);
    out+=std::stoi(msec);
    return out;
}
inline int parse_lyrics(const std::string& lyric_path, std::vector<Lyric>& l){
    if(!std::filesystem::exists(lyric_path))
        return EXIT_FAILURE;
    std::ifstream in(lyric_path);
    if(!in)
        return EXIT_FAILURE;

    std::string line;
    std::getline(in, line);

    if(line != "WEBVTT")
        return EXIT_FAILURE;

    Lyric y;
    std::string lines;

    while (std::getline(in, line)) {
        if(std::regex_match(line, is_timestamp)){
            y.end_time = parse_timestamp(line);
            y.str = lines;
            l.push_back(y);
            y = Lyric();
            lines.clear();
        }
        else if (line.length() != 0){
            if(lines.empty())
                lines+= line;
            else{
                (lines+=' ')+= line;
            }
        }
    }
    return 0;
}