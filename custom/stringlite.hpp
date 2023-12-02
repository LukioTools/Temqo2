#pragma once

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>


class StringLite
    {
    
    public:
        std::unique_ptr<char[]>ptr = nullptr;

        void copy(const char* str){
            auto len = strlen(str)+1;
            ptr = std::make_unique<char[]>(len);
            memcpy(ptr.get(), str, len);
        }
        void copy(const std::string_view& str){
            ptr = std::make_unique<char[]>(str.length()+1);
            memcpy(ptr.get(), str.begin(), str.length()+1);
        }
        void copy(const std::string& str){
            ptr = std::make_unique<char[]>(str.length()+1);
            memcpy(ptr.get(), str.c_str(), str.length()+1);
        }
        


        void clear(){
            ptr = nullptr;
        }


        StringLite& operator=(const std::string_view& str){
            copy(str);
            return *this;
        }
        StringLite& operator=(const std::string& str) {
            copy(str);
            return *this;
        }
        StringLite& operator=(const char* str){
            copy(str);
            return *this;
        }
        StringLite& operator=(const StringLite& str){
            copy(str.ptr.get());
            return *this;
        }


        inline operator const char*() const {
            return ptr.get();
        }
        inline operator std::string() const {
            return ptr.get();
        }
        const char* get_p() const {
            return ptr.get();
        }
        std::string get_s() const{
            return ptr.get();
        }
        //just wrapped strlen
        inline size_t lenght() const{
            if(ptr.get())
                return strlen(ptr.get());
            else
                return 0;
        }
        inline size_t length() const {
            return lenght();
        }
        //just wrapped lenght
        inline size_t size() const {
            return lenght();
        }


        StringLite(/* args */) {}
        StringLite(const StringLite& strlite) {
            copy(strlite.ptr.get());
        }
        StringLite(const char* str) {
            copy(str);
        }
        StringLite(const std::string_view& str) {
            copy(str);
        }
        StringLite(const std::string& str) {
            copy(str);
        }
        ~StringLite() {}
};
