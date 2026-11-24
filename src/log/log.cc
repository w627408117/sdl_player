#include "log/log.h"
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <cstring>

Log::Log(std::string log_file_path)
{
    file_fd_ = open(log_file_path.c_str(), O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0664);
    if (file_fd_ <= 0)
    {
        std::cout << "open fail error " << strerror(file_fd_) << std::endl;
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

void Log::log_i(const std::string &tag, const std::string &info_msg)
{
    std::string msg = tag + " " + info_msg + "\n";
    write_log(msg);
}

void Log::log_d(const std::string &tag, const std::string &debug_msg)
{
    std::string msg = tag + " " + debug_msg + "\n";
    write_log(msg);
}

void Log::log_w(const std::string &tag, const std::string &warn_msg)
{
    std::string msg = tag + " " + warn_msg + "\n";
    write_log(msg);
}

void Log::log_e(const std::string &tag, const std::string &error_msg)
{
    std::string msg = tag + " " + error_msg + "\n";
    write_log(msg);
}

ssize_t Log::write_log(const std::string &msg)
{
    std::cout << msg << std::endl;
    ssize_t write_size = write(file_fd_, msg.c_str(), msg.length());
    if (write_size <= 0)
    {
        std::cout << "write log failed " << strerror(write_size) << std::endl;
    }
    return write_size;
}