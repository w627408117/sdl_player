#pragma once

#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/channel_layout.h>
}

namespace wyan
{
    namespace player
    {
        class WyanPlayer
        {
        public:
            WyanPlayer();
            ~WyanPlayer();
            int init();
            int openMediaFile(std::string file_path);
            void profileMediaFile();
            void findVideoStreamInfo();
            int openVideoCodec();
            int decodeVideo();

            void findAudioStreamInfo();
            int openAudioCodec();
            int decodeAudio();
        private:
            AVFormatContext *avFormatContext_ = nullptr;
            std::string file_path_;
            AVCodecContext *avCodecContext_ = nullptr;
            int videoIndex_ = 0;
            int audioIndex_ = 0;
        };
    }
}