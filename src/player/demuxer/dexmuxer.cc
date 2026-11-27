#include "player/demuxer/demuxer.h"
#include <unistd.h>
#include <fcntl.h>

using namespace wyan::demuxer;

Demuxer::Demuxer(const std::string &media_file_path)
{
    int ret = avformat_open_input(&av_format_context_, media_file_path.c_str(), nullptr, nullptr);
    if (ret < 0)
    {
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buffer, sizeof(err_buffer));
        LOGD("Demuxer", "open media file error, error info: %s,  error code: %d", err_buffer, ret);
    }
    LOGD("Profile", "open media file success");
}

Demuxer::~Demuxer()
{
    if (av_format_context_ != nullptr)
    {
        avformat_close_input(&av_format_context_);
        av_format_context_ = nullptr;
    }
}

int Demuxer::extract_audio(const std::string &save_video_data_file_path)
{
    if (av_format_context_ == nullptr)
    {
        LOGD("Demuxer", "open media file failed");
        return -1;
    }
    int video_file_fd = open(save_video_data_file_path.c_str(), O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0664);
    if (video_file_fd < 0)
    {
        LOGD("Demuxer", "open video data filed, erro msg: %s", strerror(errno));
        return -1;
    }

    int video_index = av_find_best_stream(av_format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0)
    {
        LOGD("Demuxer", "find video index error");
        return -1;
    }

    AVStream *video_stream = av_format_context_->streams[video_index];
    AVCodecParameters *avCodecParameters = video_stream->codecpar;
    const AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    
}

int Demuxer::extract_video(const std::string &save_audio_data_file_path)
{
    if (av_format_context_ == nullptr)
    {
        LOGD("Demuxer", "open media file failed");
        return -1;
    }
    int audio_file_fd = open(save_audio_data_file_path.c_str(), O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0664);
    if (audio_file_fd < 0)
    {
        LOGD("Demuxer", "open audio data filed, erro msg: %s", strerror(errno));
        return -1;
    }
}