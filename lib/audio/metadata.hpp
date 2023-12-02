#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <taglib/taglib.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

namespace audio
{
    namespace extra
    {
        struct AudioMetadata {
            TagLib::String title;
            TagLib::String album;
            TagLib::String artist;
            TagLib::String genre;
            TagLib::String comment;
            unsigned int track;
            unsigned int year;
        };

        inline std::optional<AudioMetadata> getMetadata(const std::string& str){

            try {
                TagLib::FileRef tag_file(str.c_str());
                auto tag = tag_file.tag();

                return AudioMetadata{
                    tag->title(),
                    tag->album(),
                    tag->artist(),
                    tag->genre(),
                    tag->comment(),
                    tag->track(),
                    tag->year(),
                };
            } catch (...) {
            }
            return std::nullopt;
        };
    }
}
