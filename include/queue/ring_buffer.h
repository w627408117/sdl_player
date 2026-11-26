#pragma once
#include <vector>
#include <cstdint>
#include <mutex>

namespace wyan
{
    namespace buffer
    {
        class RingBuffer
        {
            public:
                void push(uint8_t *data, int size);
                int pop(uint8_t *dst, int size);
            private:
                std::vector<uint8_t> buffer_;
                std::mutex mutex_;
                
        };
    };
};