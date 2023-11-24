#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <regex>
#include <string>
#include "lib/path/filename.hpp"
#include "lib/remote/remote.hpp"


int init(){
    curl  =curl_easy_init();
    if(!curl){
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    return 0;
}

int deinit(){
    curl_easy_cleanup(curl);
    return 0;
}
int main(int argc, char const *argv[])
{
    //std::string url = "em.flac";
    std::string url = "sftp://pikku@88.115.52.221/home/pikku/em.m4a";
    printf("is sftp: %s\n", std::regex_match(url, sftp_regex) ? "true" : "false");

    init();
    printf("Fetching: %s\n", url.c_str());
    fetch(url, "./");
    deinit();

    if(!std::regex_match(url, is_supportted)){
        //convertAudioToMP3(path::filename(url).c_str(), (path::filebasename(url) + "f.mp3").c_str());
        printf("Converting to: %s\n", (path::filebasename(url) + ".mp3").c_str());
        system_ffmpeg(path::filename(url), path::filebasename(url) + ".mp3");
    }
    

    return 0;
}
