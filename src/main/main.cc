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
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "WyanPlayer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // 创建渲染器（绘制按钮必须）
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // ===== 定义"打开"按钮：左上角，100x40 =====
    SDL_Rect openBtnRect = { 10, 10, 100, 40 };  // x, y, w, h
    bool openBtnHover = false;   // 鼠标悬停状态
    bool openBtnPressed = false; // 鼠标按下状态

    bool is_running = true;
    SDL_Event sdl_event;

    while (is_running) {
        // ===== 事件处理 =====
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT) {
                is_running = false;
            }

            // 鼠标移动：检测是否悬停在按钮上
            if (sdl_event.type == SDL_MOUSEMOTION) {
                int mx = sdl_event.motion.x;
                int my = sdl_event.motion.y;
                openBtnHover = (mx >= openBtnRect.x &&
                                mx < openBtnRect.x + openBtnRect.w &&
                                my >= openBtnRect.y &&
                                my < openBtnRect.y + openBtnRect.h);
            }

            // 鼠标按下：记录按下状态
            if (sdl_event.type == SDL_MOUSEBUTTONDOWN &&
                sdl_event.button.button == SDL_BUTTON_LEFT) {
                if (openBtnHover) {
                    openBtnPressed = true;
                }
            }

            // 鼠标松开：判断是否为有效点击
            if (sdl_event.type == SDL_MOUSEBUTTONUP &&
                sdl_event.button.button == SDL_BUTTON_LEFT) {
                if (openBtnPressed && openBtnHover) {
                    // ===== 按钮被点击！在这里触发"打开文件"逻辑 =====
                    std::cout << "[Button] 打开 被点击！" << std::endl;
                    // TODO: 调用你的打开文件对话框 / 解码器启动逻辑
                }
                openBtnPressed = false;  // 无论在哪松开，都重置按下状态
            }
        }

        // ===== 渲染 =====
        // 背景
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // 按钮颜色：根据状态变化
        if (openBtnPressed) {
            SDL_SetRenderDrawColor(renderer, 80, 120, 200, 255);   // 按下：深蓝
        } else if (openBtnHover) {
            SDL_SetRenderDrawColor(renderer, 100, 150, 230, 255);  // 悬停：亮蓝
        } else {
            SDL_SetRenderDrawColor(renderer, 70, 100, 160, 255);   // 默认：灰蓝
        }
        SDL_RenderFillRect(renderer, &openBtnRect);

        // 按钮边框
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &openBtnRect);

        // 呈现
        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // 约 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}  