
#pragma once
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
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
    template<typename FuncFile>
    inline int scanfn_threaded(const std::string& dir, bool recursive, FuncFile append_file){
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

            std::vector<std::thread*> threads;
            //printf("name: %s, recursive: %s, is_direcotry: %s\n", name.c_str(), recursive ? "true" : "false", std::filesystem::is_directory(name) ? "true" : "false");
            if(recursive && std::filesystem::is_directory(name)){
                scanfn_threaded(name, recursive, append_file);
            }
            else if(std::regex_match(name, supported_audio)){
                threads.push_back(new std::thread(append_file, name));
            }
            for (auto thr : threads)
            {
                if(thr)
                    thr->join();
            }
            
            free(namelist[n]);
        }
        free(namelist);
        return EXIT_SUCCESS;
    }
} // namespace audio
