#include "queue/ring_buffer.h"
#include <cstring>

using namespace wyan::buffer;

RingBuffer::~RingBuffer() {
    buffer_.clear();
}

void RingBuffer::push(uint8_t *data, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.insert(buffer_.end(), data, data + size);
}

int RingBuffer::pop(uint8_t *dst, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    int read_size = std::min(size, (int)buffer_.size());
    if(read_size > 0) {
        memcpy(dst, buffer_.data(), read_size);
        buffer_.erase(buffer_.begin(), buffer_.begin() + read_size);
    }
    return read_size;
}