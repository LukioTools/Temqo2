#include <chrono>
#include <iostream>
#include <curl/curl.h>
#include <regex>
#include <vector>
#include <string>
#include <sstream>

std::string private_key_path = "/home/pikku/.ssh/albert";
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* output) {
    size_t total_size = size * nmemb;
    output->insert(output->end(), static_cast<char*>(contents), static_cast<char*>(contents) + total_size);
    return total_size;
}

std::vector<std::string> split_string(const std::string& str,
                                      const std::string& delimiter)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + delimiter.size();
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

std::string last_word(const std::string& str){
    auto idx= str.find_last_of(' ');
    if(idx != std::string::npos){
        return str.substr(idx+1);
    }
    return str;
}


CURLcode scanremotedir(CURL* curl, std::vector<std::string>& add, const std::string& url, const char* path_to_private_key = nullptr, std::regex* weed_out_the_useless = nullptr){
    if(!curl){
        return CURLcode::CURLE_FAILED_INIT;
    }
    std::vector<char> buffer(1024);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    if(path_to_private_key){
        curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  path_to_private_key);
    }

    auto res = curl_easy_perform(curl);
    if(res){
        return res;
    }
     
    std::string str(buffer.begin(), buffer.end());
    std::vector<std::string> files = split_string(str, "\n");
    for (auto e : files) {
        if(weed_out_the_useless){
            if(!std::regex_match(e, *weed_out_the_useless)){
                continue;
            }
        }
        if(e.length() == 0){
            continue;
        }
        add.push_back(e);
    }


    return CURLcode::CURLE_OK;
}
unsigned long time_spent_fetching = 0;
CURLcode scanRemote(CURL* curl, std::string url, std::vector<std::string>& files, bool recursive = true, const char* private_key_file = nullptr, std::regex file_rgx = std::regex("^-.*$"), std::regex dir_rgx = std::regex("^d.*$")){
    if(!curl){
        return CURLcode::CURLE_FAILED_INIT;
    }
    std::vector<char> buffer(1024);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    if(private_key_file){
        curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  private_key_file);
    }
    {
        auto s = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        auto res = curl_easy_perform(curl);
        if(res){
            return res;
        }
        time_spent_fetching += std::chrono::high_resolution_clock::now().time_since_epoch().count() - s;
    }
    


    std::string buffer_2;
    for (char c : buffer) {
        if(c == '\n'){
            auto lword = last_word(buffer_2);
            if(lword.length() == 0 || lword == "." || lword == ".."){
                continue;
            }
            if(recursive && std::regex_match(buffer_2, dir_rgx) ){
                auto newpath = url+lword+"/";
                scanRemote(curl, newpath, files, recursive, private_key_file, file_rgx, dir_rgx);
            }
            else if(std::regex_match(buffer_2, file_rgx)){
                auto newpath = url+lword;
                files.push_back(newpath);
            }
            buffer_2 = "";
        }
        else{
            buffer_2+=c;
        }
        
    }

    return CURLcode::CURLE_OK;
}

int main() {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if(!curl){
        return -1;
    }
    std::vector<std::string> files;
    //std::regex file_parser("^-.*\\.m4a$");
    std::regex file_parser("^-.*\\.m4a$");
    auto s = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto vale = scanRemote(curl, "sftp://pikku@88.115.52.221/home/pikku/", files, true, private_key_path.c_str(), file_parser);
    
    auto e = std::chrono::high_resolution_clock::now().time_since_epoch().count() - s;
    //scanremotedir(curl, files, "sftp://pikku@88.115.52.221/home/pikku/", private_key_path.c_str(), &file_parser);
    
    for (auto e : files) {
        std::cout  << e << std::endl;
    }

    std::cout << "Time spent fetching: " << time_spent_fetching << std::endl;
    std::cout << "Time spent not fetchin: " << e-time_spent_fetching  << std::endl;
    std::cout << "Time spent total: " << e  << std::endl;

    return 0;
}
