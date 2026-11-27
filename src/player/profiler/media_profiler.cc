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

    int ret = avformat_find_stream_info(avformat_context_, nullptr);
    if (ret < 0)
    {
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buffer, sizeof(err_buffer));
        LOGD("Profiler", "find stream info, error info: %s,  error code: %d", err_buffer, ret);
        return -1;
    }

    LOGD("Profiler", "media file name: %s", avformat_context_->filename);

    // 封装容器中，最长的时长，可能是音频或者视频的
    int64_t media_duration = avformat_context_->duration / AV_TIME_BASE;
    LOGD("Profiler", "media duration: %d", media_duration);

    // 封装容器 码率的总和
    LOGD("Profiler", "media bit rate: %d", avformat_context_->bit_rate);

    LOGD("Profiler", "media nb stream: %d", avformat_context_->nb_streams);

    // 取这些流中最小的那个流的时间戳当作start_time
    LOGD("Profiler", "media start time: %d", avformat_context_->start_time);

    LOGD("Profiler", "media start time: %d", avformat_context_->start_time);

    LOGD("Profiler", "media info format: %s", avformat_context_->iformat->name);

    LOGD("Profiler", "media info url: %s", avformat_context_->url);

    LOGD("Profiler", "media info audio_codec_id: %d", avformat_context_->audio_codec_id);

    int video_index = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0)
    {
        LOGD("Profiler", "find video index error");
        return -1;
    }

    int audio_index = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_index < 0)
    {
        LOGD("Profiler", "find audio index error");
        return -1;
    }

    AVStream *video_stream = avformat_context_->streams[video_index];
    AVStream *audio_stream = avformat_context_->streams[audio_index];

    LOGD("Profiler", "=========================video info start===========================");
    LOGD("Profiler", "video codec id : %s", avcodec_get_name(video_stream->codecpar->codec_id));
    LOGD("Profiler", "voide codec width: %d", video_stream->codecpar->width);
    LOGD("Profiler", "voide codec height: %d", video_stream->codecpar->height);
    LOGD("Profiler", "voide codec bit rate: %d", video_stream->codecpar->bit_rate);
    LOGD("Profiler", "voide codec format: %d", video_stream->codecpar->format);
    LOGD("Profiler", "voide codec str format: %s", av_get_pix_fmt_name((enum AVPixelFormat)video_stream->codecpar->format));
    LOGD("Profiler", "=========================video info end===========================");

    LOGD("Profiler", "=========================audio info start===========================");
    LOGD("Profiler", "audio codec id : %s", avcodec_get_name(audio_stream->codecpar->codec_id));
    LOGD("Profiler", "audio codec bit rate: %d", audio_stream->codecpar->bit_rate);
    LOGD("Profiler", "audio codec format: %d", audio_stream->codecpar->format);

    enum AVSampleFormat sample_fmt = (enum AVSampleFormat)audio_stream->codecpar->format;
    const char *fmt_name = av_get_sample_fmt_name(sample_fmt);
    LOGD("Profiler", "audio codec str format: %s", fmt_name);
    LOGD("Profiler", "audio codec sample rate : %d", audio_stream->codecpar->sample_rate);

    int nb_channels = audio_stream->codecpar->channels;
    uint64_t channel_layout = audio_stream->codecpar->channel_layout;
    char layout_str[64];
    av_get_channel_layout_string(layout_str, sizeof(layout_str), nb_channels, channel_layout);
    LOGD("Profiler", "audio codec channel_layout : %s", layout_str);
    LOGD("Profiler", "audio codec frame_size : %d", audio_stream->codecpar->frame_size);
    LOGD("Profiler", "=========================audio info end===========================");

    return 0;
}