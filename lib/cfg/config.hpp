
#pragma once
#include <filesystem>
#include <iostream>
#include <regex>
#include <fstream>
#include <string>
#include <variant>
#include <vector>
#include "parsers.hpp"

namespace cfg
{
    typedef void(*ConfigCallback)(std::string input);

    struct InlineConfiguration
    {
        std::regex regex;
        ConfigCallback cb;
    };
    struct MultilineConfiguration //called every time after 
    {
        std::regex regex;
        ConfigCallback cb;
    };
    
    std::vector<InlineConfiguration> il_cfg;
    std::vector<MultilineConfiguration> ml_cfg;

    void add_config(InlineConfiguration il){
        il_cfg.push_back(il);
    }
    void add_config_inline(std::string name, ConfigCallback cb){
        il_cfg.push_back({std::regex("^ *\\[ *" + name + " *\\] *=.*$"), cb});
    }
    void add_config_multiline(std::string name, ConfigCallback cb){
        il_cfg.push_back({std::regex("^ *\\[ *" + name + " *\\].*"), cb});
    }
    void add_config(MultilineConfiguration ml){
        ml_cfg.push_back(ml);
    }
    std::regex comment_regex("^ *(#|\\/\\/).*$");

    inline void parse(std::string filename){
        if(!std::filesystem::exists(filename))
            return;
            
        std::ifstream is(filename);
        std::string line;
        size_t current_multiline_configuration = std::variant_npos;
        while (std::getline(is, line)) {
            if(line.length() == 0){
                continue;
            }
            if(std::regex_match(line,comment_regex)){
                continue;
            }
            //check if it is any of the inline configurations
            for (size_t i = 0; i < il_cfg.size(); i++)
            {
                if(std::regex_match(line, il_cfg[i].regex)){
                    if(!il_cfg[i].cb){
                        continue;
                    }
                    il_cfg[i].cb(line);
                    current_multiline_configuration = std::variant_npos;
                    goto _continue_;
                }
            }
            //check if it is any of the multiline configurations
            for (size_t i = 0; i < ml_cfg.size(); i++)
            {
                if(std::regex_match(line, ml_cfg[i].regex)){
                    current_multiline_configuration = i;
                    goto _continue_;
                }
            }
            //check if a multiline configuration is in effect
            if(current_multiline_configuration != std::variant_npos){
                if(ml_cfg[current_multiline_configuration].cb){
                    ml_cfg[current_multiline_configuration].cb(line);
                }
            }
            _continue_:
            continue;
        }
    }

} // namespace cfg

