#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <regex>
#include <string>
#include "lib/path/filename.hpp"
#include "lib/remote/remote.hpp"

CURL* curl = nullptr;
FILE* file;
std::regex ftp_regex("^ftp://.*@.*/.*$");
std::regex sftp_regex("^sftp://.*@.*/.*$");
std::regex is_supportted("^.*\\.mp3$");

std::string private_key_path = "/home/pikku/.ssh/rpi_login.pem";

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

//returns read error if could not read from file
CURLcode fetch_sftp(std::string url, std::string dest_dir){
    auto fname = path::filename(url);
    file = fopen((dest_dir + fname).c_str(), "wb");
    if(!file) {
        return CURLcode::CURLE_READ_ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
    curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  private_key_path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    return curl_easy_perform(curl);
}

CURLcode fetch_ftp(std::string url, std::string dest_dir){
auto fname = path::filename(url);
    file = fopen((dest_dir + fname).c_str(), "wb");
    if(!file) {
        return CURLcode::CURLE_READ_ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
    //curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  private_key_path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    return curl_easy_perform(curl);
}


CURLcode fetch(std::string url, std::string dest_dir){
    if(std::regex_match(url, sftp_regex)){
        return fetch_sftp(url, dest_dir);
    }
    if(std::regex_match(url, ftp_regex)){
        return fetch_ftp(url, dest_dir);
    }
    return CURLcode::CURLE_UNSUPPORTED_PROTOCOL;
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
    std::string url = "sftp://pikku@88.115.52.221/home/pikku/em.m4a";
    printf("is sftp: %s\n", std::regex_match(url, sftp_regex) ? "true" : "false");

    init();
    fetch(url, "./");
    deinit();
    

    return 0;
}
