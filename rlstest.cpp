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
        add.push_back(e);
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
    std::regex file_parser("^(d.{6,999})||(.*\\.m4a)$");

    scanremotedir(curl, files, "sftp://pikku@88.115.52.221/home/pikku/", private_key_path.c_str(), &file_parser);

    for (auto e : files) {
        std::cout << "File: " << e << std::endl;
    }

    return 0;
}
