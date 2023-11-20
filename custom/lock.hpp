


#include <atomic>
#include <queue>
#include <thread>
class Lock
{
private:
    struct lock_t
    {
        void(*cb)(void*);
        void* ptr;
    };
      

    volatile bool locked_b = false;
public:


    //returns false if locking succeeded
    inline bool lock(){
        bool state = locked_b;
        locked_b = true;
        return state;
    }
    inline bool locked(){
        return locked_b;
    }

    inline void waitForLock(){
        while (locked_b) {
            std::this_thread::sleep_for(std::chrono::microseconds(25));
        }
    }

    inline void unlock(){
        locked_b = false;
    }


    Lock() {}
    ~Lock() {}
};