#include <thread>
#include <array>
#include <cstdint>

#include "constants.hpp"

struct cache_line
{
    char padding[504]; // Ryzen 4000 series has 64 byte cache lines
    uint64_t a;
};

static std::array<cache_line, ARRAY_SIZE> data;

static auto single_threaded() -> void
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < TGT_CNT; j++)
        {
            data[i].a++;
        }
    }
}

static auto single_threaded_striding() -> void
{
    // This implementation strides over each cache line 10000 times which *should* mean that it
    // does 10000 cache evictions & loads and so is much slower.
    for (int j = 0; j < TGT_CNT; j++)
    {
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            data[i].a++;
        }
    }
}

int main()
{
    // Get each 'a' element of each cache_line struct to count up to 1000
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        data[i].a = 0;
    }

#ifdef STRIDE
    single_threaded_striding();
#else
    single_threaded();
#endif

    return 0;    
}
