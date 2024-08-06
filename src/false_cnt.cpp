#include <shared_mutex>
#include <thread>

#include "constants.hpp"

struct counters
{
    uint32_t counters[ARRAY_SIZE];
};

static counters c;

// Not a very good control but just see how fast a single thread can do this work.
static auto st() -> void
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < TGT_CNT; j++)
        {
            c.counters[i]++;
        }
    }
}

// Create 32 thread to contend over each cache line, should be perfect contention.
#define THREAD_CNT 32

static std::shared_mutex mut;

#include <iostream>

static auto thrfunc(uint32_t start, uint32_t stride) -> void
{
    mut.lock_shared();

    auto ind = start;

    while (ind < ARRAY_SIZE && c.counters[ind] == 0)
    {
        for (int i = 0; i < TGT_CNT; i++)
            c.counters[ind]++;

        ind += stride;
    }
}

static auto false_sharing() -> void
{
    mut.lock();

    std::thread ts[THREAD_CNT];

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i] = std::thread(thrfunc, i, THREAD_CNT);

    mut.unlock();

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i].join();
}

// Create 
static auto no_false_sharing() -> void
{
    mut.lock();

    std::thread ts[THREAD_CNT];

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i] = std::thread(thrfunc, (ARRAY_SIZE / THREAD_CNT) * i, 1);

    mut.unlock();

    for (int i = 0; i < THREAD_CNT; i++)
        ts[i].join();
}

int main()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
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