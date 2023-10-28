#pragma once

#include <atomic>
#include <chrono>
#include <cstdlib>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace audio
{
    ma_decoder* curr = nullptr; //shall be mallocated
    ma_device device;

    std::atomic<ma_int64> framesRead = 0;
    std::atomic<float> volume(1.0f);  // Initial volume level

    bool playing = false;

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


    inline void cb(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
        ma_decoder_read_pcm_frames(curr, pOutput, frameCount, NULL);
        //volume shenaniganse
        //yoo its plausable to manipulate the data :333 
        float* pOutputFloat = (float*)pOutput;
        for (size_t i = 0; i < frameCount * pDevice->playback.channels; ++i) {
            pOutputFloat[i] *= volume.load();  // Apply volume level
        }
        //
        framesRead.fetch_add(frameCount);
        (void)pInput;
        (void)pDevice;
    }

    inline int seek_samples(ma_int64 samples){
 
        framesRead.fetch_add(samples);
        if(framesRead.load() < 0){
            framesRead.store(0);
        }

        return ma_decoder_seek_to_pcm_frame(curr, framesRead.load(std::memory_order_relaxed));
        
    }
    template<typename t, typename n>
    inline int seek(std::chrono::duration<t,n> dur){
        auto seconds = std::chrono::seconds(dur).count();
        return seek_samples(seconds*curr->outputSampleRate);
    }



    inline int load(const char* filename){
        if(!curr)
            return -1;
        ma_decoder_init_file(filename, nullptr, curr);
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
        ma_decoder_uninit(curr);
        free(curr);
        curr = nullptr;
        return 0;
    }

    inline int init(const char* filename){
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
