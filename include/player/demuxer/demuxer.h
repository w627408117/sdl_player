#pragma once
#include "log/log.h"
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

namespace wyan
{
    namespace demuxer
    {
        class Demuxer
        {
        public:
            Demuxer(const std::string &media_file_path);
            ~Demuxer();

            int extract_audio(const std::string &save_video_data_file_path);
            int extract_video(const std::string &save_audio_data_file_path);

        private:
            AVFormatContext *av_format_context_ = nullptr;
        };
    }
}