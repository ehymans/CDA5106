#ifndef CACHE_COMPONENTS_H
#define CACHE_COMPONENTS_H

#include <vector>
#include <algorithm>
#include <queue>
#include <string>

// CacheLine class definition
class CacheLine
{
public:
    long long tag;
    bool dirty;
    bool fault;
    std::string bits;
    CacheLine() : tag(-1), dirty(false), fault(false), bits("") {}
};

// CacheSet class definition
class CacheSet
{
public:
    std::vector<CacheLine> lines;
    std::vector<long long> lru_position;
    std::queue<int> fifo_position; // this may need to be changed from an int

    CacheSet(int assoc, int block_size) : lines(assoc), lru_position(assoc)
    {
        for (int i = 0; i < assoc; ++i)
        {
            lru_position[i] = i;

            for (int j = 0; j < block_size; ++j) 
            {
                lines[i].bits += "11111111";
            }
        }
    }
};

#endif // CACHE_COMPONENTS_H