


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
      

    volatile bool locked = false;
    std::queue<lock_t> queue;
public:


    //returns false if locking succeeded
    bool lock(){
        bool state = locked;
        locked = true;
        return state;
    }

    //unlock called from other thread
    void onUnlock(lock_t lc){
        if(!locked){
            if(!lock()){
                lc.cb(lc.ptr);
                return;
            };
        }
        queue.push(lc);
    }
    //waits for unlock
    void onUnlockSync(){
        if(!locked){
            if(!lock()){
                return;
            };            
        }
        bool waitfor = true;

        onUnlock({[](void* userdata){
            *(bool*)userdata = false;
        }, &waitfor});
        while (waitfor)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(25));
        }
        
        return;
    }

    void unlock(){
        if(queue.size() == 0){
            locked = false;
            return;
        }
        lock_t l = queue.back();
        queue.pop();
        l.cb(l.ptr);
        locked = false;
    }


    Lock() {}
    ~Lock() {}
};