#pragma once

#include <chrono>
#include <regex>
#include <string>
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
    #define current_index_cfg "CurrentIndex"
    #define current_time_cfg "CurrentTime"
    /**
     * playlist file documentation
     * [Path]
     * #filepaths
     */
    std::regex playlist_attr_rgx("^ *\\[[A-Za-z]*\\] *$");
    std::regex playlist_path_rgx("^ *\\[" path_cfg "\\] *$");
    std::regex playlist_indx_rgx("^ *\\[" current_index_cfg "\\] *= *[0-9]{1,20} *$");
    std::regex playlist_time_rgx("^ *\\[" current_time_cfg "\\] *= *[0-9]{1,20} *$");

    //implement modes
    class Playlist
    {
    private:
        
    public:
        size_t current_index = 0;
        std::vector<std::string> files;
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
        std::string current(){
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

        void clear(){
            current_index = 0;
            files.clear();
            use_file = "";
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
            return time;
        }

        int save(std::string dest_file = ""){
            if(dest_file.length() == 0)
                dest_file = use_file;


            std::ofstream out(dest_file);
            if(!out){
                return -1;
            }
            out << "[" current_index_cfg "] = " << current_index << "\n";
            out << "[" current_time_cfg "] = " << currentSongPosition().count() << "\n";
            out<<"[" path_cfg "]\n";
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
