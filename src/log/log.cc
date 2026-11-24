#include "log/log.h"
#include <unistd.h>
#include <fcntl.h>

Log::Log(std::string log_file_path)
{
    file_fd_ = open(log_file_path.c_str(), O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0664);
    if (file_fd_ <= 0)
    {
        throw std::runtime_error("open log file error");
    }
}

Log::~Log()
{
    if (file_fd_ >= 0)
    {
        close(file_fd_);
        file_fd_ = -1;
    }
}

void Log::log_i(std::string tag, std::string info_msg)
{
    std::string msg = tag + " " + info_msg;
    write(file_fd_, msg.c_str(), msg.length());
}

void Log::log_d(std::string tag, std::string debug_msg)
{
    std::string msg = tag + " " + debug_msg;
    write(file_fd_, msg.c_str(), msg.length());
}

void Log::log_w(std::string tag, std::string warn_msg)
{
    std::string msg = tag + " " + warn_msg;
    write(file_fd_, msg.c_str(), msg.length());
}

void Log::log_e(std::string tag, std::string error_msg)
{
    std::string msg = tag + " " + error_msg;
    write(file_fd_, msg.c_str(), msg.length());
}