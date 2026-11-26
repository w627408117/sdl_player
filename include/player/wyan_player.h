#pragma once

#include <string>
#include "queue/ring_buffer.h"

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
            int init_sdl_audio();

            wyan::buffer::RingBuffer* get_buffer();

        private:
            AVFormatContext *avFormatContext_ = nullptr;
            std::string file_path_;
            AVCodecContext *avCodecContext_ = nullptr;
            int videoIndex_ = 0;
            int audioIndex_ = 0;
            wyan::buffer::RingBuffer *audio_buffer_ = nullptr;

        };
    }
}