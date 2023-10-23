#include <vector>
#include "row.hpp"
namespace wm
{
    
    class WindowManager
    {
    private:
        DynamicHolder<Row3*> dynamo;

        u_short current_height(){
            u_short out;
            for (auto row : dynamo.data) {
                out+=row->space.h;
            }
            return out;
        }

    public:

        Row3* add_row(DisplayMode dm, u_short h){
            auto ptr = new Row3(dm, current_height(), h);
            dynamo.append(ptr, ptr->d, ptr->space.h);
            return ptr;
        }

        WindowManager() {}
        ~WindowManager() {}
    };

} // namespace wm

