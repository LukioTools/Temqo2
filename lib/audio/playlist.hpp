#pragma once

#include <regex>
#include <string>
#include <vector>
#include <filesystem>
#include "scandir.hpp"

#include <algorithm>
#include <random>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <regex>
namespace audio
{
    /**
     * playlist file documentation
     * [Path]
     * #filepaths
     */
    std::regex playlist_attr_rgx("^ *[A-Za-z*] *$");
    std::regex playlist_path_rgx("^ *[Path] *$");

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
        int load(std::string playlist_file){
            std::ifstream in(playlist_file);
            if(!in)
                return -1;

            std::string line;
            bool add_ = true;
            while (std::getline(in, line)) {
                if(line.length() == 0 || line[0] == '#'  ){
                    continue;
                }
                boost::trim(line);
                if (line.length() > 5)
                {
                    if(line[0] == '[' && line[1] == 'P' && line[2] == 'a' && line[3] == 't' && line[4] == 'h' && line[5] == ']'){
                        add_ = true;
                    }
                }
                if(add_){
                    add(line);
                }
                

            };

            return 0;
        }

        int save(std::string dest_file){
            std::ofstream out(dest_file);
            if(!out){
                return -1;
            }
            out<<"[Path]\n";
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
