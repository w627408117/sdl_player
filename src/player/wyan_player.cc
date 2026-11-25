#include "player/wyan_player.h"
#include <SDL2/SDL.h>
#include <iostream>

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


