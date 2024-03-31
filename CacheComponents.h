#ifndef CACHE_COMPONENTS_H
#define CACHE_COMPONENTS_H

#include <vector>
#include <algorithm>
#include <queue>

// CacheLine class definition
class CacheLine
{
public:
    long long tag;
    bool dirty;
    CacheLine() : tag(-1), dirty(false) {}
};

// CacheSet class definition
class CacheSet
{
public:
    std::vector<CacheLine> lines;
    std::vector<long long> lru_position;
    std::queue<int> fifo_position; // this may need to be changed from an int

    CacheSet(int assoc) : lines(assoc), lru_position(assoc)
    {
        for (int i = 0; i < assoc; ++i)
        {
            lru_position[i] = i;
        }
    }
};

#endif // CACHE_COMPONENTS_H