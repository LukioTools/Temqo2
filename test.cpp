

#include "lib/wm/clip.hpp"
#include <codecvt>
#include <iostream>
#include <locale>
#include <ostream>
#include <string>


int main(int argc, char const *argv[])
{
    std::string str = "/home/pikku/Music/EDM/【東方Remix】ナイト・オブ・ナイツ (MRM REMIX) ⧸ ビートまりお Remixed by モリモリあつし【ナイト・オブ・ナイツ】-xv_bc0i8YvI.mp3";
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);  


    std::cout  << "str len: " << str.length() << std::endl; 
    std::cout  << "wstr len: " << wm::correct_wswidth(wstr) << std::endl;   

    wm::correct_wsubstr(wstr, 5, 55);
    
    str =  converter.to_bytes(wstr);
    std::cout << str << std::endl;

    std::cout  << "str len: " << str.length() << std::endl; 
    std::cout  << "wstr len: " << wm::correct_wswidth(wstr) << std::endl;   

    return 0;
}
