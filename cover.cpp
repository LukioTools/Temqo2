#include "lib/audio/extract_img.hpp"

int main(int argc, char const *argv[])
{
    audio::extra::extractAlbumCover("irridecent.mp3", "current.png");
    return 0;
}
