#pragma once

#include "space.hpp"
#include "window.hpp"
#include <sstream>

namespace wm
{
    std::string str_repeat(const char *c, int n)
    {
        std::ostringstream os;
        for (int i = 0; i < n; i++)
            os << c;
        return os.str();
    }

    enum SPLICE_TYPE
    {
        END_CUT,
        END_DOTS,
        BEGIN_CUT,
        BEGIN_DOTS,
    };
    typedef std::string (*wprintln_splicer)(wm::Space space, std::string str, SPLICE_TYPE st);
    inline int sprintln(wm::Space space, std::string str, SPLICE_TYPE st = SPLICE_TYPE::BEGIN_DOTS, wprintln_splicer csplicer = nullptr)
    {
        if (!space.exists())
            return -1;

        /// remove newlines, since they anyoing
        while (std::size_t idx = str.find_first_of('\n'))
        {
            if (idx == std::string::npos)
            {
                break;
            }
            try
            {
                str.erase(idx);
            }
            catch (...)
            {
                std::cout << "ioutof range";
                return -1;
            }
        }

        

        // trunctuate if it was larger than suposed to be
        if (str.length() > space.width())
        {
            switch (st)
            {
            case BEGIN_DOTS:
                str = str.substr(str.length() - space.width(), str.length());
                str.replace(0, 3, "...");
                str = str.substr(0, space.width());
                break;
            case END_DOTS:
                str = str.substr(0, space.width() - 3); //(-3 to make space for dots)
                str.append("...");
                str = str.substr(0, space.width());
                break;
            case BEGIN_CUT:
                str = str.substr(str.length() - space.width(), str.length());
                break;
            case END_CUT:
            default:
                if (csplicer)
                {
                    str = csplicer(space, str, st);
                }
                if (str.length() > space.width())
                { // just in case cap that bitch
                    str = str.substr(0, space.width());
                }
                break;
            }
        }
        else
        {
            str.append(space.width() - str.length(), ' ');
        }

        std::cout << cursor_to_column(space.x) << str;

        return 0;
    }
    

    /*prints a line whilst clipping exess*/
    inline int sprint(wm::Space wspace, std::string str)
    {
        while (std::size_t idx = str.find_first_of('\n'))
        {
            if (idx == std::string::npos)
            {
                break;
            }
            try
            {
                str.erase(idx);
            }
            catch (...)
            {
                std::cout << "ioutof range";
                return -1;
            }
        };

        size_t iteration = 0;
        size_t pos = 0;
        std::basic_string<char> s;
        while (true)
        {
            s.clear();

            s = str.substr(pos, wspace.width());
            pos += wspace.width();

            std::cout << cursor_to_column(wspace.x) << s << cursor_down(1);
            iteration++;
            if ((iteration > wspace.h) || (s.length() < wspace.w))
            {
                break;
            }
        }
        return 0;
    }

    inline int wprint(wm::Window *window, std::string str)
    {
        return sprint(window->wSpace(), str);
    };
} // namespace wm
