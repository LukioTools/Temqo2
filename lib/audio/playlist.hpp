#pragma once

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <regex>
#include <string>
#include <thread>
#include <variant>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <random>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <regex>

#include "scandir.hpp"
#include "sfml.hpp"
#include "../../clog.hpp"
#include "../../custom/stringlite.hpp"

namespace audio
{
    #define path_cfg "Path"
    #define folder_cfg "Folder"
    #define current_index_cfg "CurrentIndex"
    #define current_time_cfg "CurrentTime"
    #define shuffle_seed "ShuffleSeed"
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
    std::regex playlist_seed_rgx("^ *\\[" shuffle_seed "\\] *= *[0-9]{1,20} *$");

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
        uint_fast32_t seed = 0;

        inline bool sorted(){
            return seed == 0;
        }
        
        inline bool empty(){
            return files.size() == 0;
        }

        inline void sort(){
            std::sort(files.begin(), files.end());
            seed = 0;
        }

        //-1 doesnt exist, 1 exists but is not supportted, 0 success
        inline int add(std::string path, bool recursive = true){
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

        inline std::optional<std::string> opt_next(){
            if(!files.size())
                return std::nullopt;
            current_index++;
            if(current_index >= files.size())
                current_index = 0;
            
            return files[current_index];
        }
        inline std::optional<std::string> opt_curr(){
            if(!files.size())
                return std::nullopt;
            
            return files[current_index];
        }

        inline std::optional<std::string> opt_prev(){
            if(!files.size())
                return std::nullopt;
            if(current_index == 0)
                current_index = files.size(); //-1 needed but -- happens so we can simplify :3
            
            current_index--;
            return files[current_index];
        }

        //may throw idk man
        inline std::string next(){
            if(files.size() < 1){
                return "";
            }

            current_index++;
            if(current_index >= files.size()){
                current_index = 0;
            }
            return files[current_index];
        }
        inline std::string current(){
            if(files.size() < 1){
                return "";
            }
            return files[current_index];
        }
        //previous, loops if nececcary
        inline std::string prev(){
            if(files.size() < 1){
                return "";
            }
            if(current_index == 0){
                current_index = files.size(); //-1 needed but -- happens so we can simplify :3
            }
            current_index--;
            return files[current_index];
        }

        inline std::string find(const std::string& thing, size_t* index = nullptr){
            if(index){
                *index = std::string::npos;
            }
            for (size_t i = 0; i < files.size(); i++)
            {
                if(files[i].find(thing) != std::string::npos){
                    if(index){
                        *index = i;
                    }
                    return files[i];
                }
            }
            return "";
        }
        inline size_t find2(const std::string& thing, size_t find_skips = 0){
            auto find_lower = boost::to_lower_copy(thing);
            for (size_t i = 0; i < files.size(); i++)
            {
                auto str_lower = boost::to_lower_copy(files[i]);
                if(str_lower.find(find_lower) != std::string::npos){
                    if(find_skips-- == 0)
                        return i;
                }
            }
            return std::variant_npos;
        }
        inline size_t find2multiword(const std::string& thing, size_t find_skips = 0){
            auto find_lower = boost::to_lower_copy(thing);
            std::vector<std::string> find_items;
            boost::split(find_items,find_lower, boost::is_any_of(" "));
            
            for (size_t i = 0; i < files.size(); i++)
            {
                auto file = boost::to_lower_copy(files[i]); 
                for (auto e : find_items)
                {
                    if(file.find(e) == std::variant_npos)
                        goto continue_;
                    
                }
                if(find_skips-- > 0)
                    continue;
                return i;
                continue_:
                continue;
            }
            return std::variant_npos;
        }
        
        inline void find2allmulti(std::vector<size_t>& vec, const std::string& thing){
            auto find_lower = boost::to_lower_copy(thing);
            std::vector<std::string> find_items;
            boost::split(find_items,find_lower, boost::is_any_of(" "));

            for (size_t i = 0; i < files.size(); i++)
            {
                auto file = boost::to_lower_copy(files[i]); 
                for (auto e : find_items)
                {
                    if(file.find(e) == std::variant_npos)
                        goto continue_;
                    
                }
                vec.push_back(i);
                continue_:
                continue;
            }

        };



        inline std::string find_insensitive(const std::string& thing, size_t* index = nullptr, bool split = true){
            std::vector<std::string> find_shits;
            split ?  (void) boost::split(find_shits, thing, boost::is_any_of(" ")) : find_shits.push_back(thing);
            

            if(index)
                *index = std::string::npos;

            for (size_t i = 0; i < files.size(); i++)
            {
                for (auto check_shit: find_shits)
                {
                    auto it = std::search(
                        files[i].begin(), files[i].end(),
                        thing.begin(),   thing.end(),
                        [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
                    );
                    if(it == files[i].end()){
                        goto loop_again;
                    };
                }
                *index = i;
                return files[i];
                
                    //lööp again
                loop_again:
                continue;
            }
            
            
            return "";
        }
        //find regex
        inline std::string frgx(const std::regex& rgx, size_t* index = nullptr){
            if(index){
                *index = std::string::npos;
            }
            for (size_t i = 0; i < files.size(); i++)
            {
                if(std::regex_match(files[i], rgx)){
                    if(index){
                        *index = i;
                    }
                    return files[i];
                }
            }
            return "";
        }

        inline void shuffle(){
            if(files.size() < 1){
                return;
            }
            auto rng = std::default_random_engine(seed);
            std::shuffle(files.begin(), files.end(), rng);
        }

        inline void new_seed(){
            std::random_device rd;
            seed = rd();
        }

        inline void clear(){
            current_index = 0;
            files.clear();
            use_file = "";
        }



        inline void unique(){

            std::thread thr1(__unique, &files);
            std::thread thr2(__unique, &folders);

            thr2.join();
            thr1.join();
            return;
        }

        inline int use(std::string playlist_file){
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

            if(current_index >= files.size()){
                current_index = 0;
            }
            unique();

            return time;
        }

        int use(){
            return use(use_file);
        }

        size_t size(){
            return files.size();
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
        

        inline std::string get(size_t index){
            if( index >= files.size() || index < 0){
                return "";
            }
            return files[index];
        }
        inline std::optional<std::string> opt_get(size_t index){
            if( index >= files.size() || index < 0){
                return std::nullopt;
            }
            return files[index];
        }

        inline std::string& operator[](size_t index){
            return files[index];
        }


        Playlist() {}
        ~Playlist() {}
    };
} // namespace audio
