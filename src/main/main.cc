#include "player/wyan_player.h"
#include "log/log.h"

int main()
{

    wyan::log::Log log("../local_log/log.txt");
    wyan::player::WyanPlayer wyan_player;
    wyan_player.init();
    int open_status = wyan_player.openMediaFile("../media_test_20s.mp4");
    if (open_status < 0)
    {
        char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(open_status, err_buf, sizeof(err_buf));
        LOGE("player", "avformat_open_input failed: %s (error_code: %d)", err_buf, open_status);
        return -1;
    }

    wyan_player.profileMediaFile();

    // wyan_player.findVideoStreamInfo();

    // wyan_player.openVideoCodec();

    // wyan_player.decodeVideo();

    // wyan_player.findAudioStreamInfo();
    // wyan_player.openAudioCodec();
    // wyan_player.decodeAudio();
    return 0;
}