#include "lib/audio/playlist_metadata.hpp"
#include <iostream>
#include <iterator>


audio::PlaylistMetadata pm;

int main(int argc, char const *argv[])
{
    std::cout << "add: " << (pm.add("/home/pikku/Music/")? "true" : "false") << std::endl;

    pm.sort();

    for (auto& e : pm) {
        std::cout << "title: " << e.o_md->title.toCString(true) << std::endl;
    }

    return 0;
}
