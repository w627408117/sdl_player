#include "player/wyan_player.h"
#include "log/log.h"

int main()
{

    wyan::log::Log log("../local_log/log.txt");
    LOGD("wyan", "日志创建成功");
    LOGI("wyan", "日志创建成功");
    LOGW("wyan", "日志创建成功");
    LOGE("wyan", "日志创建成功");

    // wyan::player::WyanPlayer wyan_player;
    // wyan_player.init();

    // wyan_player.openMediaFile("/Users/cms/Downloads/test_with_audio.mp4");

    // wyan_player.profileMediaFile();

    // wyan_player.findVideoStreamInfo();

    // wyan_player.openVideoCodec();

    // wyan_player.decodeVideo();

    // wyan_player.findAudioStreamInfo();
    // wyan_player.openAudioCodec();
    // wyan_player.decodeAudio();
    return 0;
}