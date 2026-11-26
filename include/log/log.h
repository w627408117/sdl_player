#pragma once
#include <iostream>
#include <string>
#include <mutex>

namespace wyan
{
    namespace log
    {
        class Log
        {
        public:
            Log(std::string log_file_path);
            ~Log();
            static void log_i(const std::string &tag, const std::string &file_name, const char *fmt, ...);
            static void log_d(const std::string &tag, const std::string &file_name, const char *fmt, ...);
            static void log_w(const std::string &tag, const std::string &file_name, const char *fmt, ...);
            static void log_e(const std::string &tag, const std::string &file_name, const char *fmt, ...);

        private:
            static ssize_t write_log(const std::string &msg);
            static std::mutex mtx_;
            static int file_fd_;
        };
    }
}

#define LOGD(TAG, fmt, ...) wyan::log::Log::log_d(TAG, __FILE__, fmt, ##__VA_ARGS__)
#define LOGI(TAG, fmt, ...) wyan::log::Log::log_i(TAG, __FILE__, fmt, ##__VA_ARGS__)
#define LOGW(TAG, fmt, ...) wyan::log::Log::log_w(TAG, __FILE__, fmt, ##__VA_ARGS__)
#define LOGE(TAG, fmt, ...) wyan::log::Log::log_e(TAG, __FILE__, fmt, ##__VA_ARGS__)
