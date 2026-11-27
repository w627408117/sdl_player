#include "player/wyan_player.h"
#include "log/log.h"
#include <stdint.h>
#include <SDL2/SDL.h>

void player()
{
    // wyan::log::Log log("../local_log/log.txt");
    // wyan::player::WyanPlayer wyan_player;
    // wyan_player.init();

    // int open_status = wyan_player.openMediaFile("../media_test_20s.mp4");
    // if (open_status < 0)
    // {
    //     char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    //     av_strerror(open_status, err_buf, sizeof(err_buf));
    //     LOGE("player", "avformat_open_input failed: %s (error_code: %d)", err_buf, open_status);
    //     return -1;
    // }

    // wyan_player.profileMediaFile();
    // // wyan_player.findVideoStreamInfo();
    // // wyan_player.openVideoCodec();
    // // wyan_player.decodeVideo();

    // wyan_player.findAudioStreamInfo();
    // wyan_player.openAudioCodec();
    // wyan_player.init_sdl_audio();
    // wyan_player.decodeAudio();
}

int main()
{
    wyan::log::Log log("../local_log/log.txt");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOGE("Player", "SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
        "WyanPlayer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        LOGE("Player", "SDL Create Window falied: %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    bool is_running = true;
    SDL_Event sdl_event;

    while (is_running)
    {
        while (SDL_PollEvent(&sdl_event))
        {
            if (sdl_event.type == SDL_QUIT)
            {
                is_running = false;
                LOGD("Player", "SDL Quit");
            }
        }
    }

    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();
    return EXIT_SUCCESS;
}