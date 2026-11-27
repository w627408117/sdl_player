#pragma once
#include <string>
#include "log/log.h"
extern "C" {
    #include <libavformat/avformat.h>
}

namespace wyan {
    namespace profiler {
        class MediaProfiler {
            public:
                MediaProfiler();
                ~MediaProfiler();
                int load_media_file(const std::string& media_file_path);
                int profile_media();
            private:
                AVFormatContext *avformat_context_ = nullptr;
                int media_file_fd_ = -1;
        };
    }
}