#include "player/wyan_player.h"
#include <SDL2/SDL.h>
#include <iostream>
#include "log/log.h"

wyan::player::WyanPlayer::WyanPlayer()
{
}

wyan::player::WyanPlayer::~WyanPlayer()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    avformat_close_input(&avFormatContext_);
    avformat_network_deinit();
}

int wyan::player::WyanPlayer::init()
{
    // 全局初始化网络组件
    return avformat_network_init();
}

int wyan::player::WyanPlayer::openMediaFile(std::string file_path)
{
    this->file_path_ = file_path;
    return avformat_open_input(&avFormatContext_, file_path.c_str(), nullptr, nullptr);
}

void wyan::player::WyanPlayer::profileMediaFile()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    av_dump_format(avFormatContext_, 0, this->file_path_.c_str(), 0);
}

void wyan::player::WyanPlayer::findVideoStreamInfo()
{
    if (avFormatContext_ == nullptr)
    {
        return;
    }
    avformat_find_stream_info(avFormatContext_, nullptr);
    videoIndex_ = av_find_best_stream(avFormatContext_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
}

int wyan::player::WyanPlayer::openVideoCodec()
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

    std::cout << "解码器已打开: " << avCodec->name
              << "  " << avCodecContext_->width << "x" << avCodecContext_->height
              << std::endl;
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

int wyan::player::WyanPlayer::decodeVideo() {
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
                        LOGD("wyan", "receive_frame AVERROR(EAGAIN) || ret == AVERROR_EOF");
                        break;
                    }

                    if (ret < 0)
                    {
                        LOGD("wyan", "receive_frame error");
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

