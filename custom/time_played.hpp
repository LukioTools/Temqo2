#include "../lib/wm/element.hpp"
#include <initializer_list>
#include <vector>

class TimePlayed : public wm::Element
{
private:
    void bounds(){
        if(index >= states.size()){
            index = 0;
        }
    }
public:
    struct TimePlayedDraw{
        size_t alloc_size;
        void(*callback)();
    };

    std::vector<TimePlayedDraw> states;
    unsigned int index;
    bool hover;


    TimePlayedDraw current(){
        if(!exists()){
            return {0,nullptr};
        }
        bounds();
        return states[index];
    }

    TimePlayedDraw next(){
        this->operator++();
        if(!exists()){
            return {0,nullptr};
        }
        bounds();
        return states[index];
    }

    TimePlayedDraw prev(){
        this->operator--();
        if(!exists()){
            return {0,nullptr};
        }
        bounds();
        return states[index];
    }

    void operator--(){
        if(!exists()){
            index = 0;
            return;
        }
        if(index == 0){
            index = static_cast<size_t>(states.size()-1);
            return;
        }
        index--;
    }

    void operator++(){
        index++;
        bounds();
    }

    TimePlayed& operator=(wm::Space s){
        space = s;
        return *this;
    }

    TimePlayed& operator=(wm::Padding p){
        pad = p;
        return *this;
    }






    operator wm::Padding&(){
        return pad;
    }

    void add(TimePlayedDraw c){
        states.push_back(c);
    }

    bool exists(){
        return states.size() != 0UL;
    }

    TimePlayed() {}
    TimePlayed(std::initializer_list<TimePlayedDraw> ls): states(ls) {}

    ~TimePlayed() {}
};