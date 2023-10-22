#include <vector>
#include "row.hpp"
namespace wm
{
    
    class WindowManager
    {
    private:
        std::vector<Row2*> rows;

        u_short current_height(){
            u_short out;
            for (auto row : rows) {
                out+=row->h;
            }
            return out;
        }

    public:

        Row2* add_row(u_short h){
            auto ptr = new Row2(h,current_height());
            rows.push_back(ptr);
            return ptr;
        }

        WindowManager() {}
        ~WindowManager() {}
    };

} // namespace wm

