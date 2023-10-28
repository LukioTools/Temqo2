#pragma once

#include <regex>
#include <string>
#include <vector>
#include <filesystem>
#include "scandir.hpp"

#include <algorithm>
#include <random>

namespace audio
{
    //implement modes
    class Playlist
    {
    private:
        
    public:
        size_t current_index = 0;
        std::vector<std::string> files;

        //-1 doesnt exist, 1 exists but is not supportted, 0 success
        int add(std::string path, bool recursive = true){
            if(!std::filesystem::exists(path)){
                return -1;
            }
            if(std::filesystem::is_directory(path)){
                if(path[path.length()-1] != '/'){
                    path+='/';
                }
                scan(files, path,  true);
            }else if(std::regex_match(path, supported_audio)){
                files.push_back(path);
            }
            else{
                return 1;
            }
            return EXIT_SUCCESS;
        }
        //may throw idk man
        std::string next(){
            current_index++;
            if(current_index >= files.size()){
                current_index = 0;
            }
            return files[current_index];
        }
        //previous, loops if nececcary
        std::string prev(){
            if(current_index == 0){
                current_index = files.size(); //-1 needed but -- happens so we can simplify :3
            }
            current_index--;
            return files[current_index];
        }

        void shuffle(){
            auto rng = std::default_random_engine {};
            std::shuffle(files.begin(), files.end(), rng);
        }

        inline std::vector<std::string>::iterator  begin(){
            return files.begin();
        }

        inline std::vector<std::string>::iterator  end(){
            return files.end();
        }
        
        std::string operator[](size_t index){
            return files[index];
        }


        Playlist() {}
        ~Playlist() {}
    };
} // namespace audio
