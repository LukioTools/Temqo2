
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <chrono>

namespace audio
{
    sf::SoundBuffer current_buffer;
    sf::Sound player;

    //returns true if couldnt load
    inline bool load(const std::string& filename){
        auto res =  current_buffer.loadFromFile(filename);
        if(res == false){
            return true;
        }
        player.setBuffer(current_buffer);
        return false;
    }
    namespace control
    {
        inline void play(){
        if(player.getStatus() == sf::Sound::Playing){
            return;
        }
        player.play();
        return;
        }
        inline void pause(){
            player.pause();
        }
        inline void toggle(){
            if(player.getStatus() == sf::Sound::Playing){
                control::pause();
                return;
            }
            control::play();
        }
    } // namespace control
    
    
    
    namespace volume
    {
        //[0,100]
        inline float get(){
            return player.getVolume();
        }

        //[0,100]
        inline void set(float f){
            if(f < 0)
                f = 0.f;
            else if(f > 100)
                f = 100.f;

            player.setVolume(f);
        }
        inline void shift(float f){
            volume::set(volume::get()+f);
        }
    } // namespace volume

    namespace duration
    {
        template<typename ratio>
        inline std::chrono::duration<long long, ratio> get(){
            return std::chrono::duration_cast<std::chrono::duration<long long, ratio>>(std::chrono::microseconds(player.getBuffer()->getDuration().asMicroseconds()));
        }
        
    } // namespace duration
    

    namespace seek
    {
        //[0,1]
        inline void abs(double i){
            player.setPlayingOffset(sf::microseconds(duration::get<std::micro>().count()*i));
        }
        template<typename T, typename Rat>
        inline void abs(std::chrono::duration<T,Rat> d){
            std::chrono::microseconds microseconds = std::chrono::duration_cast<std::chrono::microseconds>(d);
            player.setPlayingOffset(sf::microseconds(microseconds.count()));
        }
        //seconds
        template<typename T, typename Rat>
        inline void rel(std::chrono::duration<T,Rat> d){
            std::chrono::microseconds microseconds = std::chrono::duration_cast<std::chrono::microseconds>(d);
            player.setPlayingOffset(
                sf::microseconds(
                    duration::get<std::micro>().count()+microseconds.count()
                )
            );
        }
    } // namespace seek
    
    

} // namespace audio
