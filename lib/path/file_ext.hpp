#pragma once


#include <string>
namespace path
{
    inline std::string file_ext(std::string path){
        auto idx = path.find_last_of('.');
        if(idx == std::string::npos){
            return path;
        }
        return path.substr(idx+1);
    }
} // namespace path
