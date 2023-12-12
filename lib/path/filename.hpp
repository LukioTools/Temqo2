#pragma once


#include <string>
namespace path
{
    inline std::string filename(std::string path){
        auto idx = path.find_last_of('/');
        if(idx == std::string::npos){
            return path;
        }
        return path.substr(idx+1);
    }
    inline std::string filebasename(std::string path){
        auto fname = filename(path);
        return fname.substr(0,fname.find_first_of('.'));
    }
    inline std::string pathfilebasename(std::string path){
        return path.substr(0,path.find_first_of('.'));
    }
    inline std::string fileext(std::string path){ // includes dot
        return path.substr(path.find_last_of('.'));
    }

} // namespace path
