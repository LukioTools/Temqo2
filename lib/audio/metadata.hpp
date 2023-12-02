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
#include "../../custom/stringlite.hpp"

namespace audio
{
    namespace extra
    {
        struct AudioMetadata {
            StringLite title;
            StringLite album;
            StringLite artist;
            StringLite genre;
            StringLite comment;
            unsigned int track;
            unsigned int year;
        };

        inline std::optional<AudioMetadata> getMetadata(const std::string& str){

            try {
                TagLib::FileRef tag_file(str.c_str());
                auto tag = tag_file.tag();

                return AudioMetadata{
                    tag->title().toCString(true),
                    tag->album().toCString(true),
                    tag->artist().toCString(true),
                    tag->genre().toCString(true),
                    tag->comment().toCString(true),
                    tag->track(),
                    tag->year(),
                };
            } catch (...) {
            }
            return std::nullopt;
        };
    }
}
