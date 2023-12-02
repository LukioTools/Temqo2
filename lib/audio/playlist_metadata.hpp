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
#include "sfml.hpp"

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
#define vor(el) el.o_md.value_or(audio::extra::AudioMetadata{})

        void sort(SortBy s){
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

        void sort(){
            sort(s);
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

        inline FileMetadata& get(size_t index){
            return at(index);
        }
        inline std::optional<FileMetadata> opt_get(size_t index){
            if( index >= size() || index < 0){
                return std::nullopt;
            }
            return at(index);
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
            return {};
        }

        inline FileMetadata find_insensitive(const std::string& thing, size_t* index = nullptr, bool split = true){
            std::vector<std::string> find_shits;
            split ?  (void) boost::split(find_shits, thing, boost::is_any_of(" ")) : find_shits.push_back(thing);

            if(index)
                *index = std::string::npos;


            for (size_t i = 0; i < size(); i++)
            {
                auto it = std::search(
                    at(i).file.cbegin(), at(i).file.cend(),
                    thing.begin(),   thing.end(),
                    [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
                );
                if(it != at(i).file.end()){
                    if(index)
                        *index = i;

                    return at(i);
                };
            }
            
            
            return {};
        }

        inline void unique(){

            std::thread thr([this](){
                std::sort(folders.begin(), folders.end());
                auto it = std::unique(folders.begin(), folders.end());
                folders.resize(std::distance(folders.begin(),it));
            });

            sort(SortBy::FILEPATH);
            auto it = std::unique(begin(), end());
            resize(std::distance(begin(),it));

            thr.join();
            return;
        }

        inline int use(const std::string& playlist_file){
            if(!std::filesystem::exists(playlist_file))
                return 2;
            std::ifstream in(playlist_file);
            use_file = playlist_file;
            if(!in)
                return -1;
            unsigned long long time = 0;
            std::string line;
            bool add_ = false;
            while (std::getline(in, line)) {
                if(line.length() == 0 || line[0] == '#'  ){
                    continue;
                }
                boost::trim(line);
                if (std::regex_match(line, playlist_path_rgx))
                {
                    add_ = true;
                }
                else if(std::regex_match(line, playlist_fldr_rgx)){
                    add_ = true;
                }
                else if(std::regex_match(line, playlist_attr_rgx)){
                    add_ = false;
                }

                if(std::regex_match(line, playlist_indx_rgx)){
                    
                    auto idx = line.find_first_of('=');
                    if(idx == std::string::npos){
                        continue;
                    }
                    current_index = std::stoull(line.substr(idx+1));
                }
                else if(std::regex_match(line, playlist_time_rgx)){
                    auto idx = line.find_first_of('=');
                    if(idx == std::string::npos){
                        continue;
                    }
                    time = std::stoull(line.substr(idx+1));
                }else if(std::regex_match(line, playlist_seed_rgx)){
                    auto idx = line.find_first_of('=');
                    if(idx == std::string::npos){
                        continue;
                    }
                    seed = std::stoull(line.substr(idx+1));

                }
                


                if(add_){
                    add(line);
                }
            };
        
            if(current_index >= size()){
                current_index = 0;
            }
            
            unique();

            return time;
        }

        inline int save(std::string dest_file = ""){
            if(dest_file.length() == 0)
                dest_file = use_file;

            //unique();
            std::ofstream out(dest_file);
            if(!out){
                return -1;
            }
            out << "\n[" current_index_cfg "] = " << current_index << "\n";
            out << "\n[" current_time_cfg "] = " << audio::position::get<std::ratio<1,1>>().count() << "\n";
            out << "\n[" shuffle_seed "] = " << seed << "\n";
            out<<"\n[" folder_cfg "]\n";
            for (auto e : folders)
            {
                out << e << "\n";
            }
            
            out<<"\n[" path_cfg "]\n";
            for(auto& e : *this){
                out << e.file << "\n";
            }
            out.close();

            return 0;
        }
        


        
        inline FileMetadata& operator[](size_t index){
            return at(index);
        }



        PlaylistMetadata() {}
        ~PlaylistMetadata() {}
    };

}