

#include <codecvt>
#include <iostream>
#include <locale>
#include "custom/enum.hpp"

ENUM(Enumerator, unsigned char, A, B, C, D);

int main(int argc, char const *argv[])
{
    /*
        std::string str = "/home/pikku/Music/EDM/【東方Remix】ナイト・オブ・ナイツ (MRM REMIX) ⧸ ビートまりお Remixed by モリモリあつし【ナイト・オブ・ナイツ】-xv_bc0i8YvI.mp3";
        std::cout  << "str len: " << str.length() << std::endl;


        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring wstr = converter.from_bytes(str);

        std::cout  << "wstr len: " << wstr.length() << std::endl;


        std::cout << str << std::endl;
        std::wcout << wstr << std::endl;
        str = converter.to_bytes(wstr);
        std::cout << str << std::endl;


    */

    // ENUM2(Enumerator, unsigned char, A, B, C = 5);
    Enumerator e;
    e.force(3);

    std::cout << e.get() << std::endl;
    return 0;
}
