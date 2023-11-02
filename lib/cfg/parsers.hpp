#pragma once

#include <ostream>
#include <regex>
#include <string>
namespace cfg
{
    std::string parse_inline(const std::string& str){
        return str.substr(str.find_first_of('=')+1);
    }
    
    std::string get_bracket_contents(const std::string& str, size_t* end_b_pos = nullptr, size_t offset = 0){
        auto bg_first = str.find_first_of('{', offset);
        auto bg_last = str.find_first_of('}', offset);
        
        if(bg_first == std::string::npos || bg_last == std::string::npos){
            if(end_b_pos){
                *end_b_pos = std::string::npos;
            }
            return "";
        }
        if(end_b_pos){
            *end_b_pos = bg_last;
        }
        bg_first++;
        return str.substr(bg_first, bg_last-bg_first);
    }
    struct RGB{
        unsigned char r;
        unsigned char g;
        unsigned char b;

        friend std::ostream& operator<<(std::ostream& os, const RGB& rgb){
            os << "r: " << rgb.r << " g: " << rgb.g << " b: " <<rgb.b;
            return os;
        }
    }; 
    
    RGB parse_rgb(std::string bracket_contents){
        auto first = bracket_contents.find_first_of(',');
        auto last = bracket_contents.find_last_of(',');
        if(first == std::string::npos){
            return {static_cast<unsigned char>(std::stoi(bracket_contents)),0,0};
        }
        if(last == std::string::npos){
            return {
                static_cast<unsigned char>(std::stoi(bracket_contents.substr(0, first))),
                static_cast<unsigned char>(std::stoi(bracket_contents.substr(first+1))),
                0,
            };
        }
        return {
            static_cast<unsigned char>(std::stoi(bracket_contents.substr(0, first))),
            static_cast<unsigned char>(std::stoi(bracket_contents.substr(first+1, last))),
            static_cast<unsigned char>(std::stoi(bracket_contents.substr(last+1))),
        };
    }

    std::regex parse_bool_regex("^ *(true|ye?s?|1) *$", std::regex::icase);
    bool parse_bool(std::string content){
        return std::regex_match(content, parse_bool_regex);
    }

} // namespace cfg
