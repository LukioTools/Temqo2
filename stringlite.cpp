

#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>


class StringLite
    {
    private:
        /* data */
    public:
        std::unique_ptr<char[]>ptr = nullptr;


        void clear(){
            ptr = nullptr;
        }

        StringLite& operator=(const std::string_view& str){
            ptr = std::make_unique<char[]>(str.length()+1);
            memcpy(ptr.get(), str.begin(), str.length()+1);
            return *this;
        }
        StringLite& operator=(const char* str){
            auto len = strlen(str)+1;
            ptr = std::make_unique<char[]>(len);
            memcpy(ptr.get(), str, len);
            return *this;
        }

        inline operator const char*(){
            return ptr.get();
        }
        inline operator std::string(){
            return ptr.get();
        }
        const char* get_p(){
            return ptr.get();
        }
        std::string get_s(){
            return ptr.get();
        }
        //just wrapped strlen
        size_t lenght(){
            if(ptr.get())
                return strlen(ptr.get());
            else
                return 0;
        }
        //just wrapped lenght
        size_t size(){
            return lenght();
        }


        StringLite(/* args */) {}
        ~StringLite() {

        }
} str;

int main(int argc, char const *argv[])
{
    std::cout << "hell\n";

    str = "Hello World!";

    std::cout << str << std::endl;

    return 0;
}
