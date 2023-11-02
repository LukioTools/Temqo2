#include "lib/ansi/ascii_img2.hpp"
#include "lib/cfg/config.hpp"
#include <iostream>
#include <regex>
#include <string>
#include <variant>


void config_init(){
    cfg::add_config_inline("WarnColor", [](std::string parse){
        auto str = parse.substr(parse.find_first_of('=')+1);
        size_t bpos = 0;
        auto bg = cfg::get_bracket_contents(str, &bpos);
        auto fg = cfg::get_bracket_contents(str, &bpos, bpos+1);
        std::cout << "bg: " << cfg::parse_rgb(bg) << std::endl;
        std::cout << "fg: " << cfg::parse_rgb(fg) << std::endl;
    });
    cfg::add_config_inline("Volume", [](std::string parse) {
        auto str = parse.substr(parse.find_first_of('=')+1);
        auto vol = std::stod(str);
        std::cout << "vol: " << vol << std::endl;
    });
    cfg::add_config_inline("CoverArt", [](std::string parse) {
        auto str = parse.substr(parse.find_first_of('=')+1);
        auto enable_cover = cfg::parse_bool(str);
        std::cout << "enable cover: " << enable_cover << std::endl;
    });
    
}

int main(int argc, char const *argv[])
{
    config_init();
    cfg::parse("temqo.cfg");    
    
    return 0;
}
