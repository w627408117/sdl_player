#include "player/wyan_player.h"
#include <SDL2/SDL.h>
#include <iostream>
#include "log/log.h"

using namespace wyan::player;

const static unsigned int SAMPLE_RATE = 44100;

WyanPlayer::WyanPlayer()
{
}

WyanPlayer::~WyanPlayer()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    avformat_close_input(&avFormatContext_);
    avformat_network_deinit();
    if (audio_buffer_ != nullptr)
    {
        delete audio_buffer_;
        audio_buffer_ = nullptr;
    }
}

int WyanPlayer::init()
{
    // 全局初始化网络组件
    return avformat_network_init();
}

int WyanPlayer::openMediaFile(std::string file_path)
{
    this->file_path_ = file_path;
    return avformat_open_input(&avFormatContext_, file_path.c_str(), nullptr, nullptr);
}

void WyanPlayer::profileMediaFile()
{
    if (avFormatContext_ == nullptr)
    {
        LOGD("wyan", "avFormatContext_ == nullptr return");
        return;
    }
    av_dump_format(avFormatContext_, 0, this->file_path_.c_str(), 0);
}

void WyanPlayer::findVideoStreamInfo()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    avformat_find_stream_info(avFormatContext_, nullptr);
    videoIndex_ = av_find_best_stream(avFormatContext_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
}

int WyanPlayer::openVideoCodec()
{
    if (avFormatContext_ == nullptr || videoIndex_ < 0)
    {
        return -1;
    }

    AVStream *avStream = avFormatContext_->streams[videoIndex_];
    AVCodecParameters *avCodecParameters = avStream->codecpar;
    const AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (avCodec == nullptr)
    {
        return -1;
    }

    avCodecContext_ = avcodec_alloc_context3(avCodec);
    if (!avCodecContext_)
    {
        return -1;
    }

    int ret = avcodec_parameters_to_context(avCodecContext_, avCodecParameters);
    if (ret < 0)
    {
        return ret;
    }

    avCodecContext_->thread_count = 0;
    ret = avcodec_open2(avCodecContext_, avCodec, nullptr);
    if (ret < 0)
    {
        return -1;
    }

    LOGD("player", "解码器已打开: %s,  width: %d,  height: %d", avCodec->name, avCodecContext_->width, avCodecContext_->height);
    return 0;
}

bool pump_sdl_event()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            return false;
        }
    }
    return true;
}

void render_frame(SDL_Renderer *renderer,
                  SDL_Texture *texture,
                  AVFrame *frame)
{
    SDL_UpdateYUVTexture(texture, nullptr,
                         frame->data[0], frame->linesize[0], // Y
                         frame->data[1], frame->linesize[1], // U
                         frame->data[2], frame->linesize[2]  // V
    );

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

int WyanPlayer::decodeVideo()
{
    if (!avFormatContext_ || !avCodecContext_)
    {
        return -1;
    }

    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();
    int frame_count = 0;

    // 640 x 368

    int window_width = avCodecContext_->width;
    int window_height = avCodecContext_->height;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOGD("wyan", "SDL_Init failed: %s", SDL_GetError());
        av_frame_free(&avFrame);
        av_packet_free(&avPacket);
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "FFmpeg SDL Player",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        0);

    if (!window)
    {
        LOGD("wyan", "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        av_frame_free(&avFrame);
        av_packet_free(&avPacket);
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        LOGD("wyan", "SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        av_frame_free(&avFrame);
        av_packet_free(&avPacket);
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height);

    if (!texture)
    {
        LOGD("wyan", "SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        av_frame_free(&avFrame);
        av_packet_free(&avPacket);
        return -1;
    }

    bool isRunning = true;

    while (isRunning && av_read_frame(avFormatContext_, avPacket) >= 0)
    {

        AVStream *video_stream = avFormatContext_->streams[videoIndex_];
        AVRational frame_rate = av_guess_frame_rate(avFormatContext_, video_stream, nullptr);
        double fps = av_q2d(frame_rate);
        if (fps <= 0.0)
        {
            fps = 25.0;
        }
        const int frame_delay_ms = static_cast<int>(1000.0 / fps);

        if (avPacket->stream_index == videoIndex_)
        {
            if (avcodec_send_packet(avCodecContext_, avPacket) >= 0)
            {
                while (isRunning)
                {
                    int ret = avcodec_receive_frame(avCodecContext_, avFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }

                    if (ret < 0)
                    {
                        char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
                        av_strerror(ret, err_buf, sizeof(err_buf));
                        LOGD("wyan", "receive_frame error, err_code: %d, err_msg: %s", ret, err_buf);
                        break;
                    }

                    LOGD("wyan", "frame # %d, pts= %lld, width= %d, height= %d", frame_count, avFrame->pts, avFrame->width, avFrame->height);
                    render_frame(renderer, texture, avFrame);
                    frame_count++;
                    av_frame_unref(avFrame);
                }
            }
        }
        isRunning = pump_sdl_event();
        av_packet_unref(avPacket);
        SDL_Delay(frame_delay_ms);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    avcodec_send_packet(avCodecContext_, nullptr);
    LOGD("wyan", "总解码帧数 = %d", frame_count);
    av_frame_free(&avFrame);
    av_packet_free(&avPacket);
    return frame_count;
}

void WyanPlayer::findAudioStreamInfo()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    avformat_find_stream_info(avFormatContext_, nullptr);
    audioIndex_ = av_find_best_stream(avFormatContext_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
}

int WyanPlayer::openAudioCodec()
{
    if (avFormatContext_ == nullptr || audioIndex_ < 0)
    {
        return -1;
    }

    AVStream *avStream = avFormatContext_->streams[audioIndex_];
    AVCodecParameters *avCodecParameters = avStream->codecpar;
    const AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (avCodec == nullptr)
    {
        return -1;
    }

    avCodecContext_ = avcodec_alloc_context3(avCodec);
    if (!avCodecContext_)
    {
        return -1;
    }

    int ret = avcodec_parameters_to_context(avCodecContext_, avCodecParameters);
    if (ret < 0)
    {
        return ret;
    }

    avCodecContext_->thread_count = 0;
    ret = avcodec_open2(avCodecContext_, avCodec, nullptr);
    if (ret < 0)
    {
        return -1;
    }

    LOGD("Player", "音频解码器已经打开了");
    return 0;
}

wyan::buffer::RingBuffer *WyanPlayer::get_buffer()
{
    return audio_buffer_;
}

void sdl_audio_callback(void *user_data, uint8_t *stream, int len)
{
    WyanPlayer *self = static_cast<WyanPlayer *>(user_data);
    SDL_memset(stream, 0, len);
    if (self->get_buffer())
    {
        self->get_buffer()->pop(stream, len);
    } else {
        LOGE("Player", "audio buffer error");
    }
}

int WyanPlayer::init_sdl_audio()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        LOGE("Player", "SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    LOGE("Player", "init_sdl_audio success");

    audio_buffer_ = new wyan::buffer::RingBuffer();

    SDL_AudioSpec want_spec;
    SDL_zero(want_spec);
    want_spec.freq = SAMPLE_RATE;
    want_spec.format = AUDIO_F32SYS;
    want_spec.channels = 2;
    want_spec.samples = 1024;
    want_spec.callback = sdl_audio_callback;
    want_spec.userdata = this;

    if (SDL_OpenAudio(&want_spec, nullptr) < 0)
    {
        LOGD("Player", "SDL_OpenAudio error: %s", SDL_GetError());
        return -1;
    } else {
        LOGD("Player", "open audio device success");
    }

    SDL_PauseAudio(0);
    return 0;
}

int WyanPlayer::decodeAudio()
{
    LOGD("Player", "decodeAudio");
    if (!avCodecContext_ || !avFormatContext_)
    {
        LOGD("Player", "avCodecContext 或者 avFormatContext 出现问题");
        return -1;
    }

    SwrContext *swr_context = swr_alloc();
    if (swr_context == nullptr)
    {
        LOGW("wyan", "open swr_context fail");
        avcodec_free_context(&avCodecContext_);
        avformat_close_input(&avFormatContext_);
        return -1;
    }

    av_opt_set_channel_layout(swr_context, "in_channel_layout", avCodecContext_->channel_layout, 0);
    av_opt_set_int(swr_context, "in_sample_rate", avCodecContext_->sample_rate, 0);
    av_opt_set_sample_fmt(swr_context, "in_sample_fmt", avCodecContext_->sample_fmt, 0);

    uint64_t dst_channel_layout = AV_CH_LAYOUT_STEREO;
    av_opt_set_channel_layout(swr_context, "out_channel_layout", dst_channel_layout, 0);
    av_opt_set_int(swr_context, "out_sample_rate", SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(swr_context, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

    if (swr_init(swr_context) < 0)
    {
        LOGD("wyan", "swr init fail");
        avcodec_free_context(&avCodecContext_);
        avformat_close_input(&avFormatContext_);
        swr_free(&swr_context);
        return 0;
    }

    AVFrame *audio_frame = av_frame_alloc();
    AVPacket *audio_packet = av_packet_alloc();
    if (audio_buffer_ == nullptr)
    {
        LOGD("Player", "audio queue malloc fail");
        return -1;
    }

    while (av_read_frame(avFormatContext_, audio_packet) >= 0)
    {
        if (audio_packet->stream_index == audioIndex_)
        {
            int ret = avcodec_send_packet(avCodecContext_, audio_packet);
            if (ret < 0)
            {
                LOGD("wyan", "avcodec send audio package fail");
                return -1;
            }
        }

        while (true)
        {
            int ret = avcodec_receive_frame(avCodecContext_, audio_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                return -1;
            }

            const int dst_nb_samples = ceil(audio_frame->nb_samples * ((float)SAMPLE_RATE / audio_frame->sample_rate));
            const int out_size = av_samples_get_buffer_size(NULL, 2, dst_nb_samples, AV_SAMPLE_FMT_FLT, 0);
            uint8_t *out_buf = (uint8_t *)av_malloc(out_size);
            const int out_sample_count = swr_convert(swr_context, &out_buf, dst_nb_samples, (const uint8_t **)audio_frame->data, audio_frame->nb_samples);

            if (out_sample_count < 0)
            {
                av_free(out_buf);
                av_frame_unref(audio_frame);
                continue;
            }

            int actual_out_size = out_sample_count * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);
            audio_buffer_->push(out_buf, actual_out_size);
            av_free(out_buf);
        }
        av_packet_unref(audio_packet);
        av_frame_unref(audio_frame);
    }

    avcodec_free_context(&avCodecContext_);
    avformat_close_input(&avFormatContext_);
    swr_free(&swr_context);

    return true;
}