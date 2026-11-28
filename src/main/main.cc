#include "player/wyan_player.h"
#include "log/log.h"
#include <stdint.h>
#include <SDL2/SDL.h>
#include "player/profiler/media_profiler.h"
#include "player/demuxer/demuxer.h"

int main()
{
    wyan::log::Log log("../local_log/log.txt");
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


    // wyan::profiler::MediaProfiler media_profiler;
    // media_profiler.load_media_file("../media_test_20s.mp4");
    // media_profiler.profile_media();


    wyan::demuxer::Demuxer dexmuxer("../media_test_20s.mp4");
    // dexmuxer.extract_video("../local_log/media_test_20s.yuv");
    dexmuxer.extract_audio("../local_log/media_test_20s.aac");
    return EXIT_SUCCESS;
}