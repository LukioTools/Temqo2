#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <regex>
#include <string>
#include "lib/path/filename.hpp"
#include "lib/remote/remote.hpp"

CURL* curl = nullptr;
FILE* file = nullptr;
std::regex ftp_regex("^ftp://.*@.*/.*$");
std::regex sftp_regex("^sftp://.*@.*/.*$");
std::regex is_supportted("^.*\\.mp3$");

std::string private_key_path = "/home/pikku/.ssh/albert";

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

//returns read error if could not read from file
CURLcode fetch_sftp(){
    curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
    curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  private_key_path.c_str());
    return curl_easy_perform(curl);
}

CURLcode fetch_ftp(){
    return curl_easy_perform(curl);
}


inline CURLcode fetch_init(std::string url, std::string dest_dir){
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    auto fname = path::filename(url);
    file = fopen((dest_dir + fname).c_str(), "wb");
    if(!file) {
        return CURLcode::CURLE_READ_ERROR;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    return CURLcode::CURLE_OK;    
}

CURLcode fetch(std::string url, std::string dest_dir){
    CURLcode ret = CURLcode::CURLE_UNSUPPORTED_PROTOCOL;

    {
        auto code = fetch_init(url, dest_dir);
        if(code != CURLcode::CURLE_OK){
            return code;
        }
    }

    if(std::regex_match(url, sftp_regex)){
        ret = fetch_sftp();
    }
    else if(std::regex_match(url, ftp_regex)){
        ret = fetch_ftp();
    }

    fclose(file);
    return ret;
}

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
