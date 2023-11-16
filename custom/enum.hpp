

#include <string>
#include <iostream>


#define ENUM(cname, dataType, ...)                                                    \
    class cname                                                                       \
    {                                                                                 \
        std::string convertEnumToString(dataType value) const                         \
        {                                                                             \
            static const char *enumStrings[] = {#__VA_ARGS__};                        \
            static const char *UNKNOWN = "UNKNOWN";                                   \
            int index = value - 1;                                                    \
            return (index >= 0 && index < ENUM_COUNT) ? enumStrings[index] : UNKNOWN; \
        }                                                                             \
                                                                                      \
    public:                                                                           \
        enum EnumType : dataType                                                      \
        {                                                                             \
            __VA_ARGS__,                                                              \
            ENUM_COUNT                                                                \
        };                                                                            \
        EnumType num;                                                                 \
        EnumType get() { return num; }                                                \
        void operator=(EnumType a) { this->load(a); }                                 \
        void operator=(dataType a) { this->load(a); }                                 \
        void operator=(std::string a) { this->load(a); }                              \
        std::string to_string()                                                       \
        {                                                                             \
            return convertEnumToString(num);                                          \
        }                                                                             \
        bool load(dataType a)                                                         \
        {                                                                             \
            if (a > ENUM_COUNT)                                                       \
            {                                                                         \
                return false;                                                         \
            }                                                                         \
            num = (EnumType)a;                                                        \
            return true;                                                              \
        }                                                                             \
        bool load(EnumType a)                                                         \
        {                                                                             \
            if (a > ENUM_COUNT)                                                       \
            {                                                                         \
                return false;                                                         \
            }                                                                         \
            num = a;                                                                  \
            return true;                                                              \
        }                                                                             \
        bool load(std::string a)                                                      \
        {                                                                             \
            static const char *enumStrings[] = {#__VA_ARGS__};                        \
            for (size_t i = 0; i < ENUM_COUNT; i++)                                   \
            {                                                                         \
                if (enumStrings[i] == a)                                              \
                {                                                                     \
                    num = (EnumType)i;                                                \
                    return true;                                                      \
                }                                                                     \
            }                                                                         \
            return false;                                                             \
        }                                                                             \
        cname() {}                                                                    \
        ~cname() {}                                                                   \
    }
