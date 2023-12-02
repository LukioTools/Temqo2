#pragma once

#include <chrono>
#include <iostream>
#include <iterator>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <random>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <regex>

#include "metadata.hpp"
#include "scandir.hpp"
#include "../../custom/stringlite.hpp"

namespace audio
{
    #define path_cfg "Path"
    #define folder_cfg "Folder"
    #define current_index_cfg "CurrentIndex"
    #define current_time_cfg "CurrentTime"
    #define shuffle_seed "ShuffleSeed"

    static std::regex playlist_attr_rgx("^ *\\[[A-Za-z]*\\] *$");
    static std::regex playlist_path_rgx("^ *\\[" path_cfg "\\] *$");
    static std::regex playlist_fldr_rgx("^ *\\[" folder_cfg "\\] *$");
    static std::regex playlist_indx_rgx("^ *\\[" current_index_cfg "\\] *= *[0-9]{1,20} *$");
    static std::regex playlist_time_rgx("^ *\\[" current_time_cfg "\\] *= *[0-9]{1,20} *$");
    static std::regex playlist_seed_rgx("^ *\\[" shuffle_seed "\\] *= *[0-9]{1,20} *$");


    struct FileMetadata 
    {
        std::string file = "";
        std::optional<extra::AudioMetadata> o_md = std::nullopt;
    };

    class PlaylistMetadata : public std::vector<FileMetadata>
    {
    private:
        
    public:
        
        
        size_t current_index = 0;
        ;
        std::vector<std::string> folders;
        std::string use_file;
        uint_fast32_t seed = 0;

        bool add(const std::string& dir, bool recursive = true){
            static auto add_file = [this](const std::string& str){
                push_back({
                    str,
                    extra::getMetadata(str),
                });
            };
            if(!std::filesystem::exists(dir)){
                return true;
            }
            else if(!std::filesystem::is_directory(dir)){
                add_file(dir);
                return false;
            }
            folders.push_back(dir);
            return EXIT_SUCCESS != scanfn_threaded(dir, recursive, add_file);
        }






        PlaylistMetadata() {}
        ~PlaylistMetadata() {}
    };

}