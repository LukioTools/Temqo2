#pragma once

#include <regex>
#include <string>
#include <vector>
#include <filesystem>
#include "scandir.hpp"

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
