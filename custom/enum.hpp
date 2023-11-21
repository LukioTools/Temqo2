

#include <string>
#include <iostream>
#include <map>

#define ENUM(cname, dataType, ...)                                                    \
    class cname                                                                       \
    {                                                                                 \
    public:                                                                           \
        enum EnumType : dataType                                                      \
        {                                                                             \
            __VA_ARGS__,                                                              \
            ENUM_COUNT                                                                \
        };                                                                            \
        EnumType num;                                                                 \
        EnumType get() { return num; }                                                \
        void operator=(EnumType a) { this->force(a); }                                \
        void operator=(dataType a) { this->force(a); }                                \
                                                                                      \
        void force(EnumType a) { this->num = a; }                                     \
        void force(dataType a) { this->num = (EnumType)a; }                           \
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
                                                                                      \
        bool operator==(EnumType a) { return this->get() == a; }                      \
        bool operator==(dataType a) { return this->get() == (EnumType)a; }            \
                                                                                      \
        cname(EnumType a) { this->force(a); }                                         \
        cname(dataType a) { this->force(a); }                                         \
        cname() {}                                                                    \
        ~cname() {}                                                                   \
    }

