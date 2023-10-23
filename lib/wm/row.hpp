#include <cmath>
#include <sys/types.h>
#include <vector>
#if !defined(C_ROW)
#define C_ROW
#include "window.hpp"
#include "globals.hpp"

namespace wm
{

    class Row
    {
    private:
        
    public:
        void refresh(Space* s, Padding* p,DisplayMode dm) {
            
        }
        std::vector<Window*> windows;
        Row() {}
        ~Row() {}
    };

} // namespace wm



#endif // C_ROW
