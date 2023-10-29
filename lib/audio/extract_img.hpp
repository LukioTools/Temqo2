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
#include "../wm/def.hpp"

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
                    std::cout << mimeType << "\n";
                    if (bitmap) {
                        // Specify the output image file name and format (e.g., "output.jpg")

                        // Save the image to a file
                        FreeImage_Save(FIF_PNG, bitmap, out.c_str(), 0);

                        std::cout << "Album cover saved to: " << out << std::endl;

                        FreeImage_Unload(bitmap);
                        return 0;
                    }
                    else{
                        std::cout << "No bitmap\n";
                    }
                }
            }
            return 1;
        }

        inline std::string getImg(std::string filename, int x, int y){
            std::ostringstream str;
            auto r = ascii_img::load_image(filename, x, y);
            for (size_t i = 0; i < r->size(); i++)
            {
                auto c = r->get(i);
                str << color_bg((int) c.r, (int) c.g, (int) c.b) << ' ' << attr_reset;
            }
            

            delete r;
            return str.str();
        } 

        inline std::string get(std::string mp3,int x, int y){
            auto ret = extractAlbumCoverTo(mp3, TMP_OUT);
            if(ret != 0){
                return "";
            }
            return getImg(TMP_OUT, x,y);
        }
    } // namespace extra
    
    
} // namespace audio
