#pragma once

#include <bits/chrono.h>
#include <chrono>
#include <iostream>
#include <vlcpp/Instance.hpp>
#include <vlcpp/vlc.hpp>

namespace audio_vlc
{
    VLC::Instance instance;
    VLC::MediaPlayer mediaPlayer;
    VLC::Media currentMedia;

    void medChanged(VLC::MediaPtr medptr){
        std::cout<< "Media Changed\n";
        mediaPlayer.play();
    }

    
    inline int load(const char* path){
        currentMedia = VLC::Media(instance, path, VLC::Media::FromPath);
        if(!currentMedia){
            return -1;
        }
        mediaPlayer.setMedia(currentMedia);
        return 0;
    }

    namespace lenght
    {
        inline std::chrono::milliseconds get_ms(){
            return std::chrono::milliseconds(mediaPlayer.length());        
        }

        inline std::chrono::seconds get_s(){
            return std::chrono::duration_cast<std::chrono::seconds>(lenght::get_ms());
        }

        inline double get_s_d(){
            return static_cast<double>(mediaPlayer.length())/1000.0;
        }

    } // namespace lenght
    

    
    

    namespace played
    {
        //0->1
        inline double get(){
            return mediaPlayer.position();
        }

        inline std::chrono::milliseconds get_ms(){
            auto delta =  mediaPlayer.length()*played::get();
            return std::chrono::milliseconds((size_t) delta);
        }

        inline std::chrono::seconds get_s(){
            return std::chrono::duration_cast<std::chrono::seconds>(played::get_ms());
        }
    } // namespace played
    
    inline bool playing(){
        return mediaPlayer.isPlaying();
    }
    
    

    namespace seek
    {
        inline void abs(double f){
            if(f > 1) f=1;
            if(f < 0) f=0;
            if(!mediaPlayer.isSeekable()) return;
            mediaPlayer.setPosition( f);
        }
        template<typename T, typename R>
        inline void abs(std::chrono::duration<T, R> f){
            std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(f);
            seek::abs(s.count()/lenght::get_s_d());
        }
        inline void rel(double f){
            seek::abs(played::get()+f);
        }
        template<typename T, typename R>
        inline void rel(std::chrono::duration<T, R> f){
            std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(f);
            seek::abs(played::get()+s.count()/lenght::get_s_d());
        }
    } // namespace seek
    
    namespace volume
    {
        inline int get(){
            return mediaPlayer.volume();
        }
        inline bool set(int i){
            return mediaPlayer.setVolume(i);
        }
        inline bool shift(int i){
            return volume::set(volume::get()+i);
        }
    } // namespace volume
    


    inline bool play(){
        return mediaPlayer.play();
    }

    inline void stop(){
        mediaPlayer.stop();
    }

    inline void toggle_play(){
        mediaPlayer.isPlaying() ? mediaPlayer.stop() : (void) mediaPlayer.play();
    }

    namespace on
    {
        inline void end(void(*fn)(void)){
            mediaPlayer.eventManager().onEndReached(fn);
        }

        inline void time(void(*fn)(libvlc_time_t idk_what_this_does)){
            mediaPlayer.eventManager().onTimeChanged(fn);
        }
    } // namespace on
    
    




    inline int init(){
        instance = VLC::Instance(0, nullptr);
        mediaPlayer = VLC::MediaPlayer(instance);
        mediaPlayer.eventManager().onMediaChanged(medChanged);
        return 0;
    }

    //is empty
    inline int deinit(){
        return 0;
    }
    
} // namespace audio_vlc
