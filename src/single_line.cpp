#include <shared_mutex>
#include <thread>

using counter_type = uint64_t;

#define TGT_CNT UINT64_MAX

#define RYZEN_CACHE_LINE_SIZE 64
#define NUM_COUNTERS (RYZEN_CACHE_LINE_SIZE / sizeof(counter_type))

struct counters
{
    counter_type counters[NUM_COUNTERS]; // 64 bytes in a line on AMD Ryzen 4000 series
};

static counters c;

// Not a very good control but just see how fast a single thread can do this work.
// static auto st() -> void
// {
//     for (int i = 0; i < ARRAY_SIZE; i++)
//     {
//         for (int j = 0; j < TGT_CNT; j++)
//         {
//             c.counters[i]++;
//         }
//     }
// }

// One thread per counter
#define THREAD_CNT NUM_COUNTERS

static std::shared_mutex mut;

static auto thrfunc(uint32_t ind) -> void
{
    mut.lock_shared();

    for (int i = 0; i < TGT_CNT; i++)
        c.counters[ind]++;
}

static auto false_sharing() -> void
{
    mut.lock();

    std::thread ts[THREAD_CNT];

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i] = std::thread(thrfunc, i);

    mut.unlock();

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i].join();
}

// Create 
static auto no_false_sharing() -> void
{
    for (int i = 0; i < NUM_COUNTERS; i++)
    {
        for (int j = 0; j < TGT_CNT; j++)
        {
            c.counters[i]++;
        }
    }
}

int main()
{
    for (int i = 0; i < NUM_COUNTERS; i++)
    {
        c.counters[i] = 0;
    }

#ifdef SHARE
    false_sharing();
#else
    #ifdef ST
        st();
    #else
        no_false_sharing();
    #endif
#endif

    return 0;
}
