#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <taglib/taglib.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

#include <FreeImage.h>  // You need to have FreeImage installed.

#include "../ansi/ascii_img2.hpp"

#define TMP_OUT "tmp.png"

namespace audio
{
    namespace extra
    {
        inline int extractAlbumCoverTo(std::string filename, std::string out){
            TagLib::MPEG::File mp3File(filename.c_str());
            TagLib::ID3v2::Tag* id3v2Tag = mp3File.ID3v2Tag();

            if (id3v2Tag) {
                // Check if the MP3 file has an album cover
                TagLib::ID3v2::FrameList coverArtFrames = id3v2Tag->frameList("APIC");
                
                if (!coverArtFrames.isEmpty()) {
                    // Extract the first album cover
                    TagLib::ID3v2::AttachedPictureFrame* coverArtFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(coverArtFrames.front());

                    // Check the MIME type to determine the image format (e.g., "image/jpeg" or "image/png")
                    std::string mimeType = coverArtFrame->mimeType().to8Bit(true);
                    
                    // Get the raw image data
                    TagLib::ByteVector coverArtData = coverArtFrame->picture(); 
                    // You can now save this image data to a file or process it as needed
                                // Save the image to a file using FreeImage
                    FIMEMORY* memory = FreeImage_OpenMemory((BYTE*) coverArtData.data(), coverArtData.size());
                    FIBITMAP* bitmap = FreeImage_LoadFromMemory(FIF_PNG, memory, 0);
                    if (bitmap) {
                        // Specify the output image file name and format (e.g., "output.jpg")

                        // Save the image to a file
                        FreeImage_Save(FIF_PNG, bitmap, out.c_str(), 0);


                        FreeImage_Unload(bitmap);
                        return 0;
                    }
                    else{
                    }
                }
            }
            return 1;
        }

        inline ascii_img::load_image_t* getImg(std::string filename, unsigned int x, unsigned int y){
            auto r = ascii_img::load_image(filename, x, y);
            if(r->size() != x*y){
                delete r;
                return nullptr;
            }
            return r;
        } 

        inline ascii_img::load_image_t* get(std::string mp3,int x, int y){
            auto ret = extractAlbumCoverTo(mp3, TMP_OUT);
            if(ret != 0){
                return nullptr;
            }
            return getImg(TMP_OUT, x,y);
        }
    } // namespace extra
    
    
} // namespace audio
