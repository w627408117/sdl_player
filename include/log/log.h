#pragma once
#include <iostream>
#include <string>

class Log {
    public:
        Log(std::string log_file_path);
        ~Log();
        void log_i(std::string tag, std::string info_msg);
        void log_d(std::string tag, std::string debug_msg);
        void log_w(std::string tag, std::string warn_msg);
        void log_e(std::string tag, std::string error_msg);
    private:
        int file_fd_ = -1;
};