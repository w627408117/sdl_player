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

int Demuxer::extract_video(const std::string &save_video_data_file_path)
{
    LOGD("Demuxer", "extract_video");
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
    if (avCodec == nullptr)
    {
        LOGD("Demuxer", "avCodec error");
        return -1;
    }

    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    if (avCodecContext == nullptr)
    {
        LOGD("Demuxer", "avCodecContext error");
        return -1;
    }

    int ret = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    if (ret < 0)
    {
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buffer, sizeof(err_buffer));
        LOGD("Demuxer", "avcodec_parameters_to_context error: %s,  error code: %d", err_buffer, ret);
        return -1;
    }

    avCodecContext->thread_count = 0;
    ret = avcodec_open2(avCodecContext, avCodec, nullptr);
    if (ret < 0)
    {
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buffer, sizeof(err_buffer));
        LOGD("Demuxer", "avcodec_open2 error: %s,  error code: %d", err_buffer, ret);
        return -1;
    }

    LOGD("Demuxer", "get video codec name : %s", avCodec->name);

    AVPacket *av_packet = av_packet_alloc();
    AVFrame *av_frame = av_frame_alloc();
    if (!av_packet || !av_frame)
    {
        LOGD("Demuxer", "alloc packet or frame failed");
        return -1;
    }

    while (av_read_frame(av_format_context_, av_packet) >= 0)
    {
        if (av_packet->stream_index == video_index)
        {
            int ret = avcodec_send_packet(avCodecContext, av_packet);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            {
                LOGD("Demuxer", "avcodec_send_packet error");
                break;
            }

            while (true)
            {
                ret = avcodec_receive_frame(avCodecContext, av_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    LOGD("Demuxer", "avcodec_receive_frame error");
                    break;
                }

                LOGD("Demuxer", "decode frame success, pts: %lld, format: %d", av_frame->pts, av_frame->format);
                int y_size = av_frame->width * av_frame->height;
                int uv_size = y_size / 4;
                write(video_file_fd, av_frame->data[0], y_size);
                for (int i = 0; i < av_frame->height / 2; i++)
                {
                    write(video_file_fd, av_frame->data[1] + i * av_frame->linesize[1], av_frame->width / 2);
                }

                for (int i = 0; i < av_frame->height / 2; i++)
                {
                    write(video_file_fd, av_frame->data[2] + i * av_frame->linesize[2], av_frame->width / 2);
                }

                av_frame_unref(av_frame);
            }
        }

        av_packet_unref(av_packet);
    }

    av_packet_free(&av_packet);
    av_frame_free(&av_frame);
    avcodec_free_context(&avCodecContext);
    close(video_file_fd); // 别忘了关闭 Linux 文件描述符
    return 0;
}

int Demuxer::extract_audio(const std::string &save_audio_data_file_path)
{
    LOGD("Demuxer", "extract_audio");
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

    return 0;
}