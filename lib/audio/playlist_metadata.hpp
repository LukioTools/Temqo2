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
#include "../path/filename.hpp"

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
        StringLite file = "";
        std::optional<extra::AudioMetadata> o_md = std::nullopt;
    };

    

    struct SortBy
    {
        static const char* SortByStrings[];

        enum Enum: unsigned char{
            FILEPATH,
            FILENAME,
            ARTIST,
            ALBUM,
            GENRE,
            YEAR,
            OUTSIDE_INDEX
        } num = FILEPATH;

        std::string toString(){
            if(num >= OUTSIDE_INDEX){
                return "";
            }
            return SortByStrings[num];
        }
        
        SortBy(){};
        SortBy(SortBy::Enum n) :num(n){};
        SortBy(const std::string& str){
            for (size_t i = 0; i < Enum::OUTSIDE_INDEX-1; i++)
            {
                if(str == SortByStrings[i]){
                    num = static_cast<Enum>(i);
                    return;
                }
            }
        };

    };

    const char* SortBy::SortByStrings[] =  {
        "FILENAME",
        "FILEPATH",
        "ARTIST",
        "ALBUM",
        "GENRE",
        "YEAR",
    };

    

    

    

    class PlaylistMetadata : public std::vector<FileMetadata>
    {
    private:
        
    public:
        
        
        
        size_t current_index = 0;
        std::vector<std::string> folders;
        std::string use_file;
        uint_fast32_t seed = 0;
        SortBy s = SortBy::Enum::FILEPATH;



        inline bool empty(){
            return size() == 0;
        }

        inline bool sorted(){
            return seed == 0;
        }

//yea it do be wilding doe
#define vor(el) el.o_md.value_or(audio::extra::AudioMetadata{"","","","","",0,0})

        void sort(){
            switch (s.num) {
            case SortBy::ARTIST:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return vor(a).artist < vor(b).artist;
                });
                break;
            case SortBy::ALBUM:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return vor(a).album < vor(b).album;
                });
                break;
            case SortBy::GENRE:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return vor(a).genre < vor(b).genre;
                });
                break;
            case SortBy::YEAR:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return vor(a).year < vor(b).year;
                });
                break;
            case SortBy::FILENAME:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return path::filename(a.file) < path::filename(b.file);
                });
                break;
            case SortBy::FILEPATH:
            default:
                std::sort(this->begin(), this->end(), [](FileMetadata& a, FileMetadata& b){
                    return a.file < b.file;
                });
                break;
            }
        }

#undef vor

        bool add(const std::string& dir, bool recursive = true){
            static auto add_file = [this](const std::string& str){
                push_back(FileMetadata{
                    str, extra::getMetadata(str)
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

        inline std::optional<FileMetadata> opt_next(){
            if(empty())
                return std::nullopt;
            current_index++;
            if(current_index >= size())
                current_index = 0;
            
            return at(current_index);
        }

        inline std::optional<FileMetadata> opt_prev(){
            if(empty())
                return std::nullopt;
            if(current_index == 0)
                current_index = size(); //-1 needed but -- happens so we can simplify :3
            
            current_index--;
            return at(current_index);
        }

        inline std::optional<FileMetadata> opt_curr(){
            if(empty())
                return std::nullopt;
                
            return at(current_index);
        }

        inline FileMetadata curr(){
            if(size() == 0){
                return FileMetadata{"", std::nullopt};
            }
            return at(current_index);
        }
        inline FileMetadata next(){
            if(size() == 0){
                return FileMetadata{"", std::nullopt};
            }
            current_index++;
            if(current_index >= size()){
                current_index = 0;
            }
            return at(current_index);
        }
        inline FileMetadata prev(){
            if(size() == 0){
                return FileMetadata{"", std::nullopt};
            }
            if(current_index == 0){
                current_index = size();
            }
            current_index--;
            return at(current_index);
        }

        inline FileMetadata find(const std::string& fname, size_t* index = nullptr){
            if(index)
                *index = std::string::npos;
            for (size_t i = 0; i < size(); i++)
            {
                auto e = at(i);
                if(std::string(e.file).find(fname) != std::string::npos){
                    if(index){
                        *index = i;
                    }
                    return e;
                }
            }
            return FileMetadata{"", std::nullopt};
        }

        




        PlaylistMetadata() {}
        ~PlaylistMetadata() {}
    };

}