
#include "lib/remote/remote.hpp"



#include <iostream>
#include <vector>
int main(int argc, char const *argv[])
{
 
    std::string url = "sftp://pikku@88.115.52.221/home/pikku/";

    r_init();

    std::string dest;


    std::vector<std::string> files;
    auto s = std::chrono::high_resolution_clock::now().time_since_epoch();

    fetch_dir(url, files);

    std::cout << "Fetch took: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() - s).count() << " ms\n";
    std::cout << files[0] << std::endl;
    r_deinit();

    return 0;
}
