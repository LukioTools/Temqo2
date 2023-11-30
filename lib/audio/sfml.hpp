#pragma once
#include <SFML/Audio/Music.hpp>
#include <chrono>

namespace audio
{


    sf::Music player;
    //returns true if couldnt load
    inline bool load(const std::string& filepath){
        return player.openFromFile(filepath);
    }
    inline sf::Music::Status status(){
        return player.getStatus();
    }
    inline bool playing(){
        return status() == sf::Music::Status::Playing;
    }
    inline bool paused(){
        return status() == sf::Music::Status::Paused;
    }
    inline bool stopped(){
        return status() == sf::Music::Status::Stopped;
    }

    namespace stats{
        inline unsigned int sampleRate(){
            return player.getSampleRate();
        }
        inline unsigned int channelCount(){
            return player.getChannelCount();
        }
    }

    namespace control
    {
        inline void play(){
            if(status() == sf::Music::Playing){
                return;
            }
            player.play();
            return;
        }
        inline void pause(){
            player.pause();
        }
        inline void toggle(){
            if(status() == sf::Music::Playing){
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
            return std::chrono::duration_cast<std::chrono::duration<long long, ratio>>(std::chrono::microseconds(player.getDuration().asMicroseconds()));
        }
        
    } // namespace duration

    namespace position{
        template<typename ratio>
        inline std::chrono::duration<long long, ratio> get(){
            return std::chrono::duration_cast<std::chrono::duration<long long, ratio>>(std::chrono::microseconds(player.getPlayingOffset().asMicroseconds()));
        }
        //[0,1]
        inline double get_d(){
            return static_cast<double>(player.getPlayingOffset().asMicroseconds())/static_cast<double>(player.getDuration().asMicroseconds());
        }
    }
    

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
        //not done
        template<typename T, typename Rat>
        inline void rel(std::chrono::duration<T,Rat> d){
            std::chrono::microseconds microseconds = std::chrono::duration_cast<std::chrono::microseconds>(d);
            player.setPlayingOffset(
                sf::microseconds(
                    position::get<std::micro>().count()+microseconds.count()
                )
            );
        }
    } // namespace seek
    
    

} // namespace audio
