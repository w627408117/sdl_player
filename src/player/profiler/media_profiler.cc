#include "player/profiler/media_profiler.h"
#include <cstring>

using namespace wyan::profiler;

MediaProfiler::MediaProfiler()
{
}

MediaProfiler::~MediaProfiler()
{
    if (avformat_context_ != nullptr)
    {
        avformat_close_input(&avformat_context_);
        avformat_context_ = nullptr;
    }
}

int MediaProfiler::load_media_file(const std::string &media_file_path)
{
    int ret = avformat_open_input(&avformat_context_, media_file_path.c_str(), nullptr, nullptr);
    if (ret < 0)
    {
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buffer, sizeof(err_buffer));
        LOGD("Profiler", "open media file error, error info: %s,  error code: %d", err_buffer, ret);
    }
    LOGD("Profile", "open media file success");

    return 0;
}

int MediaProfiler::profile_media()
{
    if (avformat_context_ == nullptr)
    {
        LOGD("Profiler", "media file not open!");
        return -1;
    }
    LOGD("Profiler", "media file name: %s", avformat_context_->filename);
    
    //封装容器中，最长的时长，可能是音频或者视频的
    int64_t media_duration =  avformat_context_->duration / AV_TIME_BASE;
    LOGD("Profiler", "media duration: %d", media_duration);

    //封装容器 码率的总和
    LOGD("Profiler", "media bit rate: %d", avformat_context_->bit_rate);


    LOGD("Profiler", "media nb stream: %d", avformat_context_->nb_streams);

    //取这些流中最小的那个流的时间戳当作start_time
    LOGD("Profiler", "media start time: %d", avformat_context_->start_time);

    return 0;
}