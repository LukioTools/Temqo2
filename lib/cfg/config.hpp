
#pragma once
#include <filesystem>
#include <iostream>
#include <regex>
#include <fstream>
#include <string>
#include <variant>
#include <vector>

#define CONFIG_REGEX_PREPEND "^ *\\[ *"
#define CONFIG_REGEX_APPEND " *\\] *=.*$"
#define ADD_CONFIG_INLINE(name, cb) cfg::il_cfg.push_back({std::regex(CONFIG_REGEX_PREPEND name CONFIG_REGEX_APPEND), cb})
#define ADD_CONFIG_MULTILINE(name, cb) cfg::ml_cfg.push_back({std::regex(CONFIG_REGEX_PREPEND name CONFIG_REGEX_APPEND), cb})

namespace cfg
{
    typedef void(*ConfigCallback)(const std::string& line);

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

    inline void add_config(InlineConfiguration il){
        il_cfg.push_back(il);
    }
    inline void add_config_inline(const std::string& name, ConfigCallback cb){
        il_cfg.push_back({std::regex("^ *\\[ *" + name + " *\\] *=.*$"), cb});
    }
    inline void add_config_multiline(const std::string&  name, ConfigCallback cb){
        ml_cfg.push_back({std::regex("^ *\\[ *" + name + " *\\].*"), cb});
    }
    inline void add_config(MultilineConfiguration ml){
        ml_cfg.push_back(ml);
    }
    std::regex comment_regex("^ *(#|\\/\\/).*$");

    inline void parse(const std::string& filename){
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

