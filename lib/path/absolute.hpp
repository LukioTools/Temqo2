#pragma once

#include <string>

namespace path
{
    inline std::string waveline(std::string path){
        auto idx = path.find_first_of('~');
        if(idx == std::string::npos){
            return path;
        }

        auto env = getenv("HOME");
        if(!env){
            env = (char*) "";
        }
        return path.replace(idx, 1, env);
    }

} // namespace path
