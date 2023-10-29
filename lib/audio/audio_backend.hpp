#pragma once

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ratio>
#include <thread>
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#include "miniaudio.h"

namespace audio
{
    ma_decoder* curr = nullptr; //shall be mallocated
    ma_device device;

    std::atomic<ma_int64> framesRead = 0;
    ma_uint64 framesInSong = 0;
    std::atomic<float> volume(1.0f);  // Initial volume level

    bool playing = false;

    bool songEnded = false;
    void(*songEndedCallback)(void);
    void(*song_played_secondly)(void);

    bool songEndedThreadRun = true;
    std::chrono::duration song_check_break_time = std::chrono::milliseconds(30);
    std::thread* thr;

    inline std::chrono::seconds currentSongPosition(){
        size_t t = framesRead / curr->outputSampleRate;
        return std::chrono::seconds(t);
    }

    void songEndedChecker(){
        auto start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        while (songEndedThreadRun) {
            if(songEnded){
                if(songEndedCallback) songEndedCallback();
                songEnded = false;
            }
            if(song_played_secondly &&  std::chrono::high_resolution_clock::now().time_since_epoch().count()-start > 100000000L){
                start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                song_played_secondly();
            }
            
            std::this_thread::sleep_for(song_check_break_time);
        }
    }






    inline void vol(float multiplier){
        audio::volume.store(audio::volume.load()*multiplier);
    }

    inline void vol_shift(float val){
        audio::volume.store(audio::volume.load()+val);
    }


    inline int play(){
        if (ma_device_start(&device) != MA_SUCCESS) {
            ma_device_uninit(&device);
            ma_decoder_uninit(curr);
            return -1;
        }
        playing = true;
        return 0;
    }

    inline int stop(){
        if(ma_device_stop(&device) != MA_SUCCESS){
            ma_device_uninit(&device);
            ma_decoder_uninit(curr);
            return -1;
        }
        playing = false;
        return 0;
    }

    inline unsigned int seconds_in_current(){
        if(!curr){
            return 0;
        }
        return framesInSong/curr->outputSampleRate;
    }


    inline void cb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
        ma_uint64 cycle_framesRead;
        ma_decoder_read_pcm_frames(curr, pOutput, frameCount, &cycle_framesRead);
        //volume shenaniganse
        //yoo its plausable to manipulate the data :333 
        float* pOutputFloat = (float*)pOutput;
        for (size_t i = 0; i < frameCount * pDevice->playback.channels; ++i) {
            pOutputFloat[i] *= volume.load();  // Apply volume level
        }

        if(frameCount != cycle_framesRead){
            songEnded = true;
        }
        else {
            songEnded = false;
        }


        //
        framesRead.fetch_add(frameCount);
        (void)pInput;
        (void)pDevice;
    }

    inline int seek_samples(ma_int64 samples){
        stop();
        framesRead.fetch_add(samples);
        if(framesRead.load() < 0){
            framesRead.store(0);
        }
        auto res = ma_decoder_seek_to_pcm_frame(curr, framesRead.load(std::memory_order_relaxed));
        play();
        return res;
        
    }
    template<typename t, typename n>
    inline int seek(std::chrono::duration<t,n> dur){
        auto seconds = std::chrono::seconds(dur).count();
        return seek_samples(seconds*curr->outputSampleRate);
    }

    inline int seek_abs_samples(ma_int64 samples){
        stop();
        framesRead.store(samples);
        auto res = ma_decoder_seek_to_pcm_frame(curr, samples);
        play();
        return res;
    }

    template<typename t, typename n>
    inline int seek_abs(std::chrono::duration<t,n> dur){
        auto seconds = std::chrono::seconds(dur).count();
        return seek_abs_samples(seconds*curr->outputSampleRate);
    }





    inline int load(const char* filename){
        if(!curr)
            return -1;
        ma_decoder_init_file(filename, nullptr, curr);
        ma_decoder_get_length_in_pcm_frames(curr, &framesInSong);
        framesRead = 0;
        return 0;
    }

    inline int load_next(const char* filename, bool _play = true){
        if(!curr)
            return -1;

        stop();

        ma_decoder_uninit(curr);
        load(filename);
        if(_play)
            play();
        return 0;
    }

    

    inline int deinit(){
        if(curr){
            ma_decoder_uninit(curr);
            free(curr);
        }
        curr = nullptr;

        songEndedThreadRun = false;
        if(thr){

            if(thr->joinable())
                thr->join();
            delete thr;
            thr = nullptr;
        }
        

        return 0;
    }

    inline int init(const char* filename){
        songEnded = false;
        songEndedThreadRun = true;
        thr = new std::thread(songEndedChecker);

        curr = (ma_decoder*) malloc(sizeof(ma_decoder));

        if(load(filename)){return -1;};

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = curr->outputFormat;
        deviceConfig.playback.channels = curr->outputChannels;
        deviceConfig.sampleRate        = curr->outputSampleRate;
        deviceConfig.dataCallback      = cb;

        if(ma_device_init(NULL, &deviceConfig, &device)){
            deinit();
            return -1;
        }
        return 0;
    }



    


} // namespace audio
