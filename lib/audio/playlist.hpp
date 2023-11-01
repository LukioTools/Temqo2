#pragma once

#include <chrono>
#include <iterator>
#include <regex>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include "audio_backend.hpp"
#include "scandir.hpp"

#include <algorithm>
#include <random>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <regex>
namespace audio
{
    #define path_cfg "Path"
    #define folder_cfg "Folder"
    #define current_index_cfg "CurrentIndex"
    #define current_time_cfg "CurrentTime"
    /**
     * playlist file documentation
     * [Path]
     * #filepaths
     */
    std::regex playlist_attr_rgx("^ *\\[[A-Za-z]*\\] *$");
    std::regex playlist_path_rgx("^ *\\[" path_cfg "\\] *$");
    std::regex playlist_fldr_rgx("^ *\\[" folder_cfg "\\] *$");
    std::regex playlist_indx_rgx("^ *\\[" current_index_cfg "\\] *= *[0-9]{1,20} *$");
    std::regex playlist_time_rgx("^ *\\[" current_time_cfg "\\] *= *[0-9]{1,20} *$");

    inline void __unique(std::vector<std::string>* vec){
        std::sort(vec->begin(), vec->end());
        auto it = std::unique(vec->begin(), vec->end());
        vec->resize(std::distance(vec->begin(), it));
    }
    //implement modes
    class Playlist
    {
    private:
        
    public:
        size_t current_index = 0;
        std::vector<std::string> files;
        std::vector<std::string> folders;
        std::string use_file;

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
                folders.push_back(path);
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
            if(files.size() < 1){
                return "";
            }
            current_index++;
            if(current_index >= files.size()){
                current_index = 0;
            }
            return files[current_index];
        }
        std::string current(){
            if(files.size() < 1){
                return "";
            }
            return files[current_index];
        }
        //previous, loops if nececcary
        std::string prev(){
            if(files.size() < 1){
                return "";
            }
            if(current_index == 0){
                current_index = files.size(); //-1 needed but -- happens so we can simplify :3
            }
            current_index--;
            return files[current_index];
        }

        std::string find(const std::string& thing,size_t* index = nullptr){
            if(index){
                *index = std::string::npos;
            }
            for (size_t i = 0; i < files.size(); i++)
            {
                if(files[i].find(thing)!= std::string::npos){
                    if(index){
                        *index = i;
                    }
                    return files[i];
                }
            }
            return "";
            
        }

        void shuffle(){
            if(files.size() < 1){
                return;
            }
            auto rng = std::default_random_engine {};
            std::shuffle(files.begin(), files.end(), rng);
        }

        void clear(){
            current_index = 0;
            files.clear();
            use_file = "";
        }



        void unique(){
            //std::sort(files.begin(), files.end());
            //std::sort(folders.begin(), folders.end());
            //auto it_files = std::unique(files.begin(), files.end());
            //auto it_folders = std::unique(folders.begin(), folders.end());
//
            //files.resize(std::distance(files.begin(), it_files));
            //folders.resize(std::distance(folders.begin(), it_folders));

            std::thread thr1(__unique, &files);
            std::thread thr2(__unique, &folders);

            thr2.join();
            thr1.join();
            return;
        }

        int use(std::string playlist_file){
            std::ifstream in(playlist_file);
            use_file = playlist_file;
            if(!in)
                return -1;
            unsigned long long time;
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
                }
                


                if(add_){
                    add(line);
                }
            };

            if(current_index >= files.size()){
                current_index = 0;
            }
            unique();

            return time;
        }

        int save(std::string dest_file = ""){
            if(dest_file.length() == 0)
                dest_file = use_file;

            //unique();
            


            std::ofstream out(dest_file);
            if(!out){
                return -1;
            }
            out << "\n[" current_index_cfg "] = " << current_index << "\n";
            out << "\n[" current_time_cfg "] = " << currentSongPosition().count() << "\n";
            out<<"\n[" folder_cfg "]\n";
            for (auto e : folders)
            {
                out << e << "\n";
            }
            
            out<<"\n[" path_cfg "]\n";
            for(auto e : files){
                out << e << "\n";
            }
            out.close();

            return 0;
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
