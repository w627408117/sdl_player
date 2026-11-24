#pragma once
#include <iostream>
#include <string>

class Log {
    public:
        Log(std::string log_file_path);
        ~Log();
        void log_i(const std::string& tag, const std::string& info_msg);
        void log_d(const std::string& tag, const std::string& debug_msg);
        void log_w(const std::string& tag, const std::string& warn_msg);
        void log_e(const std::string& tag, const std::string& error_msg);
    private:
        ssize_t write_log(const std::string& msg);
        int file_fd_ = -1;
};