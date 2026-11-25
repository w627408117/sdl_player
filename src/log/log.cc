#include "log/log.h"
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <sys/syscall.h>

std::mutex mtx_;


wyan::log::Log::Log(std::string log_file_path)
{
    file_fd_ = open(log_file_path.c_str(), O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0664);
    if (file_fd_ <= 0)
    {
        std::cout << "open fail error " << strerror(file_fd_) << std::endl;
    }
}

wyan::log::Log::~Log()
{
    if (file_fd_ >= 0)
    {
        close(file_fd_);
        file_fd_ = -1;
    }
}

void wyan::log::Log::log_i(const std::string &tag, const std::string &info_msg)
{
    std::string msg = tag + " " + info_msg;
    write_log(msg);
}

void wyan::log::Log::log_d(const std::string &tag, const std::string &debug_msg)
{
    std::string msg = tag + " " + debug_msg;
    write_log(msg);
}

void wyan::log::Log::log_w(const std::string &tag, const std::string &warn_msg)
{
    std::string msg = tag + " " + warn_msg;
    write_log(msg);
}

void wyan::log::Log::log_e(const std::string &tag, const std::string &error_msg)
{
    std::string msg = tag + " " + error_msg;
    write_log(msg);
}

ssize_t wyan::log::Log::write_log(const std::string &msg)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    std::time_t now = tv.tv_sec;

    struct tm local_time;
    localtime_r(&now, &local_time);
    char time_buf[32];
    int len = std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &local_time);
    std::snprintf(time_buf + len, sizeof(time_buf) - len, ".%03ld", tv.tv_usec / 1000);
    pid_t tid = static_cast<pid_t>(syscall(SYS_gettid));
    std::string str_log = "[" + std::string(time_buf) + "] [TID:" + std::to_string(tid) + "] " + msg + "\n";
    std::cout << str_log;
    
    std::lock_guard<std::mutex> lock(mtx_);
    ssize_t write_size = write(file_fd_, str_log.c_str(), str_log.length());
    if (write_size <= 0)
    {
        std::cerr << "write log failed " << strerror(errno) << std::endl;
    }
    return write_size;
}