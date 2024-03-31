#ifndef CACHE_COMPONENTS_H
#define CACHE_COMPONENTS_H

#include <vector>
#include <algorithm>

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
    std::vector<int> fifo_position; // this may need to be changed from an int

    CacheSet(int assoc) : lines(assoc), lru_position(assoc), fifo_position(assoc, 0) 
    {
        for(int i = 0; i < assoc; ++i) {
            lru_position[i] = i;
        }
    }
};

#endif // CACHE_COMPONENTS_H