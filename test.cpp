

#include <codecvt>
#include <iostream>
#include <locale>
#include <ostream>
#include <string>

#include <thread>
#include <mutex>

std::mutex m;

void ehh(){
    std::lock_guard<std::mutex> lock(m);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "ehh finished\n";
}


int main(int argc, char const *argv[])
{
    std::thread thr(ehh);

    std::cout << "Starting\n";
    
    
    {
        if(m.try_lock())
        std::lock_guard<std::mutex> lock(m);
        std::cout << "le main\n";
    }


    thr.join();
        std::cout << "le exit\n";
    return 0;
}
