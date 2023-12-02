#pragma once

#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


class StringLite
    {
    
    public:
        std::unique_ptr<char[]>ptr = nullptr;


        inline void copy(const char* str) noexcept {
            auto len = strlen(str)+1;
            ptr = std::make_unique<char[]>(len);
            memcpy(ptr.get(), str, len);
        }
        inline void copy(const std::string_view& str) noexcept {
            ptr = std::make_unique<char[]>(str.length()+1);
            memcpy(ptr.get(), str.begin(), str.length()+1);
        }
        inline void copy(const std::string& str) noexcept {
            ptr = std::make_unique<char[]>(str.length()+1);
            memcpy(ptr.get(), str.c_str(), str.length()+1);
        }
        


        void clear() noexcept {
            ptr = nullptr;
        }

        inline char* begin() noexcept {
            return ptr.get();
        }
        inline char* end() noexcept {
            auto b = begin();
            std::advance(b, size());
            return b;
        }
        inline const char* cbegin() const noexcept{
            return ptr.get();
        }
        inline const char* cend() const noexcept{
            auto b = cbegin();
            std::advance(b, size());
            return b;
        }

        inline StringLite& operator=(const std::string_view& str) noexcept{
            copy(str);
            return *this;
        }
        inline StringLite& operator=(const std::string& str)  noexcept{
            copy(str);
            return *this;
        }
        inline StringLite& operator=(const char* str) noexcept{
            copy(str);
            return *this;
        }
        inline StringLite& operator=(const StringLite& str) noexcept{
            copy(str.ptr.get());
            return *this;
        }


        inline operator const char*() const  noexcept {
            return ptr.get();
        }
        inline operator std::string() const  noexcept {
            return ptr.get();
        }
        inline const char* get_p() const  noexcept {
            return ptr.get();
        }
        inline std::string get_s() const noexcept {
            return ptr.get();
        }
        //just wrapped strlen
        inline size_t lenght() const noexcept {
            if(ptr.get())
                return strlen(ptr.get());
            else
                return 0;
        }
        inline size_t length() const noexcept {
            return lenght();
        }
        //just wrapped lenght
        inline size_t size() const noexcept {
            return lenght();
        }


        StringLite(/* args */) {}
        StringLite(const StringLite& strlite) noexcept {
            copy(strlite.ptr.get());
        }
        StringLite(const char* str) noexcept {
            copy(str);
        }
        StringLite(const std::string_view& str) noexcept {
            copy(str);
        }
        StringLite(const std::string& str) noexcept {
            copy(str);
        }
        ~StringLite() {}
};
