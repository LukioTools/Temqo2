#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <vector>
std::string private_key_path = "/home/pikku/.ssh/albert";

// Callback function to handle response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<std::pair<std::string, bool>>* output) {
    size_t total_size = size * nmemb;
    std::string line(static_cast<char*>(contents), total_size);

    // Tokenize the line to extract the filename and check if it is a folder
    std::istringstream iss(line);
    std::string filename;
    bool is_folder = false;
    iss >> std::skipws; if (is_folder){ iss >> std::ws; }else{iss >> filename;} ; // Skip leading whitespaces if it's a folder

    if (!filename.empty()) {
        output->emplace_back(filename, is_folder);
    }

    return total_size;
}

int main() {
    // Initialize libcurl
    CURL* curl;
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (res != CURLE_OK) {
        std::cerr << "curl_global_init() failed: " << curl_easy_strerror(res) << std::endl;
        return 1;
    }

    // Create a CURL object
    curl = curl_easy_init();
    if (curl) {
        // Set the FTP URL
        const char* url = "sftp://pikku@88.115.52.221/home/pikku/";

        // Set the callback function to handle response data
        std::vector<std::pair<std::string, bool>> files;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &files);

        // Set FTP options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE,  private_key_path.c_str());
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1L);  // Only retrieve filenames

        // Perform the FTP request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Print the list of filenames and their types
            std::cout << "List of files:\n";
            for (const auto& file : files) {
                if (file.second) {
                    std::cout << "[Folder] ";
                } else {
                    std::cout << "[File] ";
                }
                std::cout << file.first << std::endl;
            }
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Cleanup libcurl
    curl_global_cleanup();

    return 0;
}
