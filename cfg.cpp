#include "lib/ansi/ascii_img2.hpp"
#include "lib/cfg/config.hpp"
#include <iostream>
#include <regex>
#include <string>
#include <variant>

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
ascii_img::RGB<> parse_rgb(std::string bracket_contents){
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


void config_init(){
    cfg::add_config_inline("WarnColor", [](std::string parse){
        auto str = parse.substr(parse.find_first_of('=')+1);
        size_t bpos = 0;
        auto bg = get_bracket_contents(str, &bpos);
        auto fg = get_bracket_contents(str, &bpos, bpos+1);
        std::cout << "bg: " << parse_rgb(bg) << std::endl;
        std::cout << "fg: " << parse_rgb(fg) << std::endl;
    });
    cfg::add_config_inline("Volume", [](std::string parse) {
        auto str = parse.substr(parse.find_first_of('=')+1);
        auto vol = std::stod(str);
        std::cout << "vol: " << vol << std::endl;
    });
    cfg::add_config_inline("CoverArt", [](std::string parse) {
        auto str = parse.substr(parse.find_first_of('=')+1);
        auto enable_cover = parse_bool(str);
        std::cout << "enable cover: " << enable_cover << std::endl;
    });
    
}

int main(int argc, char const *argv[])
{
    config_init();
    cfg::parse("temqo.cfg");    
    
    return 0;
}
