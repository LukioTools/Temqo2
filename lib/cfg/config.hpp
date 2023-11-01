
#pragma once
#include <regex>
#include <fstream>
#include <string>

namespace cfg
{
    
    std::regex option_rgx("^ *[.*].*$");
    #define volume_rgx_name "Volume"
    std::regex volume_rgx("^ *[ *" volume_rgx_name " *] *=.*$");
    #define sens_scrl_rgx_name "ScrlSens"
    std::regex sens_scrl_rgx("^ *[ *" sens_scrl_rgx_name " *] *=.*$");
    //std::regex sens_scrl_rgx("^ *[ *ScrlSens *] *=.*$");


    template<typename vol_t>
    void parse(std::string filename, vol_t* volume){
        std::ifstream is(filename);
        std::string line;
        while (std::getline(is, line)) {
            
        }
    }

} // namespace cfg

