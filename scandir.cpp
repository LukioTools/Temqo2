#include "lib/audio/scandir.hpp"
#include <iostream>
#include <regex>

int main(int argc, char const *argv[])
{
    auto rgx =  std::regex("^.*\\.cpp$");
    auto ret = audio::scanfnrgxthr("./old/", false, rgx, [](std::string str){
        std::cout << str << std::endl;
    });

    std::cout << "ret: " << ret << std::endl;
    return 0;
}
