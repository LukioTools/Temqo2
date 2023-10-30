#pragma once



#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
inline int system_ffmpeg(std::string i, std::string o){
    if(std::filesystem::exists(o)){
        return -1;
    }
    return system(("ffmpeg -nostats -loglevel panic -hide_banner -i " + i + " " + o).c_str());
}

#include <sndfile.h>
#include <lame/lame.h>

inline int convertAudioToMP3(const char* inputFilename, const char* outputFilename) {
    // Initialize the input file
    SF_INFO input_info = {};
    SNDFILE* infile = sf_open(inputFilename, SFM_READ, &input_info);
    if (!infile) {
        std::cerr << "Failed to open input file: " << inputFilename << std::endl;
        return 1;
    }
    std::cout << "hello" << std::endl;

    // Initialize the MP3 output file
    lame_global_flags* lame = lame_init();
    lame_set_in_samplerate(lame, input_info.samplerate);
    lame_set_out_samplerate(lame, input_info.samplerate);
    lame_set_num_channels(lame, input_info.channels);
    lame_set_brate(lame, 128); // Bitrate (e.g., 128 kbps)
    lame_set_quality(lame, 2); // 2 is good quality, 7 is low quality
    lame_set_mode(lame, STEREO);
    lame_set_VBR(lame, vbr_default);
    lame_set_findReplayGain(lame, 1);

    lame_init_params(lame);

    FILE* outfile = fopen(outputFilename, "wb");
    if (!outfile) {
        std::cerr << "Failed to open output file." << std::endl;
        sf_close(infile);
        lame_close(lame);
        return 1;
    }

    // Initialize buffer for reading audio data
    const int buffer_size = 8192;
    std::vector<short> buffer(buffer_size * input_info.channels);

    while (true) {
        int read_count = sf_readf_short(infile, buffer.data(), buffer_size);
        if (read_count <= 0) {
            break;
        }

        int write_count = lame_encode_buffer_interleaved(lame, buffer.data(), read_count, (unsigned char*) buffer.data(), buffer_size);
        if (write_count > 0) {
            fwrite(buffer.data(), sizeof(short), write_count, outfile);
        }
    }


    int flush_count = lame_encode_flush(lame, (unsigned char*) buffer.data(), buffer_size);
    if (flush_count > 0) {
        fwrite(buffer.data(), sizeof(short), flush_count, outfile);
    }

    // Clean up
    fclose(outfile);
    sf_close(infile);
    lame_close(lame);

    std::cout << "Audio conversion to MP3 successful." << std::endl;

    return 0;
}
