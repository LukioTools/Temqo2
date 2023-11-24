#pragma once



#include <boost/algorithm/string/trim.hpp>
#include <cstdlib>
#include <curl/curl.h>
#include <curl/easy.h>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include "../path/filename.hpp"

inline int system_ffmpeg(std::string i, std::string o){
    if(std::filesystem::exists(o)){
        return -1;
    }
    return system(("ffmpeg -nostats -loglevel panic -hide_banner -i " + i + " " + o).c_str());
}


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
static size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *str){
    static_cast<std::stringstream*>(str)->write((const char*) ptr, nmemb);
    return nmemb;
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


CURLcode fetch_dir(const std::string& url, std::vector<std::string>& files, std::regex valid_files = std::regex(".*")){
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    std::stringstream str;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
    curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);  // Only retrieve filenames


    CURLcode ret = CURLcode::CURLE_OK;

    if(std::regex_match(url, sftp_regex)){
        ret = fetch_sftp();
    }
    else if(std::regex_match(url, ftp_regex)){
        ret = fetch_ftp();
    }
    else{
        ret = curl_easy_perform(curl);
    }

    //while (std::getline(str, line)) {
    //    std::cout << "Line: " << line << std::endl;
    //    
    //    
    //    auto file = line.substr(line.find_last_of(' ')); 
    //    if(line.size() > 0)
    //        if(line[0] == 'd')
    //            file=+'/';

    //    boost::trim(file);


    //    std::cout << "Parsed Line: " << file << std::endl;
    //    if(std::regex_match(file, valid_files))
    //        files.push_back(file);
    //}
    files.push_back(str.str());

    return ret;
}


inline CURLcode fetch_init(std::string url, std::string dest_dir, std::string* dest_file = nullptr){
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    

    auto fname = path::filename(url);
    auto fdest = (dest_dir + fname);
    if(dest_file)
        *dest_file = fdest; 

    file = fopen(fdest.c_str(), "wb");
    if(!file) {
        return CURLcode::CURLE_READ_ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    return CURLcode::CURLE_OK;
}


CURLcode fetch(std::string url, std::string dest_dir = "./cache/", std::string* dest_file = nullptr){
    CURLcode ret = CURLcode::CURLE_UNSUPPORTED_PROTOCOL;

    {
        auto code = fetch_init(url, dest_dir, dest_file);
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

int r_init(){
    curl =  curl_easy_init();
    #if defined (VERBOSE) 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #else
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    #endif

    return 0;
}

int r_deinit(){
    curl_easy_cleanup(curl);
    return 0;
}