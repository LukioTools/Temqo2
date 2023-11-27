
#pragma once
#include <cstring>
#include <regex>
#include <string>
#include <vector>
#include <dirent.h>
#include <filesystem>
namespace audio
{
    std::regex supported_audio("^.{1,255}\\.(mp3|wav|flac|ogg)$");

    inline int scan(std::vector<std::string>& vec ,std::string dir, bool recursive){
        struct dirent **namelist;

        int n = scandir(dir.c_str(), &namelist, NULL, nullptr);
        if (n == -1) {
            return EXIT_FAILURE;
        }
        while (n--) {
            if(strcmp(namelist[n]->d_name, ".") == 0 || strcmp(namelist[n]->d_name, "..") == 0 ){free(namelist[n]); continue;}
            auto name = (dir+namelist[n]->d_name);
            if(std::filesystem::is_directory(name)){
                name+='/';
            }
            

            //printf("name: %s, recursive: %s, is_direcotry: %s\n", name.c_str(), recursive ? "true" : "false", std::filesystem::is_directory(name) ? "true" : "false");
            if(recursive && std::filesystem::is_directory(name)){
                scan(vec, name, recursive);
            }
            else if(std::regex_match(name, supported_audio)){
                vec.push_back(name);
            }
            free(namelist[n]);
        }
        free(namelist);
        return 0;
    }
} // namespace audio
