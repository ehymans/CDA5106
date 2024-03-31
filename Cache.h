#ifndef CACHE_H
#define CACHE_H
// Your code here
#include "CacheComponents.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <unordered_map>

using namespace std;

class Cache
{
private:
    std::vector<CacheSet> sets;
    unsigned long long num_sets;
    unsigned int assoc;
    unsigned int block_size;
    unsigned int replacement_policy;
    unsigned int inclusion_policy;

    unsigned long long hit_count = 0;
    unsigned long long miss_count = 0;
    unsigned long long reads_count = 0;
    unsigned long long writes_count = 0;

    // additional output stats
    unsigned long long read_misses = 0;
    unsigned long long write_misses = 0;
    unsigned long long writebacks = 0;
    // bool writeback_flag = false;

public:
    Cache(unsigned int size, unsigned int assoc, unsigned int block_size, unsigned int replacement, unsigned int inclusion) : assoc(assoc), block_size(block_size), replacement_policy(replacement), inclusion_policy(inclusion)
    {
        num_sets = size == 0 ? 0 : size / (block_size * assoc);
        sets.resize(num_sets, CacheSet(assoc));
    }

    // Public methods for Cache operations
    unsigned long long getNumSets() const { return num_sets; }
    unsigned int getAssoc() const { return assoc; }
    unsigned int getBlockSize() const { return block_size; }

    bool evict_block(int set_index, int block_index);

    void update_lru(int set_index, int accessed_index);
    void update_fifo(int set_index, int index);

    // bool allocate_block(int set_index, long long tag, bool evicted_block_dirty);
    bool allocate_block(int set_index, long long tag, char op);

    bool simulate_access(char op, long long address);

    void L1_print_statistics();
    void L2_print_statistics();

    void print_contents();
    int calculate_set_index(long long address);
    long long calculate_tag(long long address);

    int find_lru_block(int set_index);
    bool writeback_flag;
    bool eviction_flag;

    long long evicted_address;

    void handle_writeback(long long address);
    bool l2_miss_on_l1_eviction(char op, long long address);
    bool hit_miss_simulate(char op, long long address);

    bool L2_simulate_access(char op, long long address);
};

int Cache::calculate_set_index(long long address)
{
    int log_block_size = static_cast<int>(log2(block_size));
    return (address >> log_block_size) % num_sets;
}

long long Cache::calculate_tag(long long address)
{
    int log_block_size = static_cast<int>(log2(block_size));
    int log_num_sets = static_cast<int>(log2(num_sets));
    return address >> (log_block_size + log_num_sets);
}

int Cache::find_lru_block(int set_index)
{
    // Assuming lru_position stores indices of blocks in their LRU order,
    // with the first element being the LRU block.
    if (!sets[set_index].lru_position.empty())
    {
        return sets[set_index].lru_position.front();
    }
    else
    {
        // Handle the case where there might not be any blocks (should not happen in a well-initialized cache)
        return -1; // Return an invalid index if for some reason the set is empty
    }
}

bool Cache::evict_block(int set_index, int block_index)
{
    bool wasDirty = sets[set_index].lines[block_index].dirty;
    evicted_address = (sets[set_index].lines[block_index].tag << (static_cast<int>(log2(block_size)) + static_cast<int>(log2(num_sets)))) | (set_index << static_cast<int>(log2(block_size)));

    eviction_flag = true; // flag so that the Simulation class knows if an eviction occurred.

    if (wasDirty)
    {
        // Increment the write-back counter
        writebacks++;
        writeback_flag = true;
    }

    // Reset the block
    sets[set_index].lines[block_index].tag = -1;
    sets[set_index].lines[block_index].dirty = false;

    return wasDirty; // returns if the block was dirty or not so that it can be written back to L2
}

void Cache::update_lru(int set_index, int accessed_index)
{
    // Move the accessed block to the most recently used position
    auto accessed_lru = std::find(sets[set_index].lru_position.begin(), sets[set_index].lru_position.end(), accessed_index);
    if (accessed_lru != sets[set_index].lru_position.end())
    {
        sets[set_index].lru_position.erase(accessed_lru);
    }
    sets[set_index].lru_position.push_back(accessed_index);
}

void Cache::update_fifo(int set_index, int index)
{
    // remove first element in queue
    sets[set_index].fifo_position.pop();

    // Place previous element at the back
    sets[set_index].fifo_position.push(index);
}

bool Cache::allocate_block(int set_index, long long tag, char op)
{
    bool foundEmptyLine = false;
    for (int i = 0; i < assoc; ++i)
    {
        if (sets[set_index].lines[i].tag == -1)
        { // Empty line found
            sets[set_index].lines[i].tag = tag;
            sets[set_index].lines[i].dirty = (op == 'w'); // Set dirty if it's a write
            update_lru(set_index, i);                     // Move to the most recently used position
            sets[set_index].fifo_position.push(i);        // Add index to fifo queue
            foundEmptyLine = true;
            break;
        }
    }

    if (!foundEmptyLine)
    {
        // Find the least recently used (LRU) block if the set is full
        if (replacement_policy == 0)
        {
            int lru_index = sets[set_index].lru_position.front();

            // Evict the LRU block
            evict_block(set_index, lru_index);

            // allocate new block
            sets[set_index].lines[lru_index].tag = tag;
            sets[set_index].lines[lru_index].dirty = (op == 'w'); // Set dirty based on operation

            // Since we just used this block, we need to update its LRU position
            update_lru(set_index, lru_index);
        }
        else if (replacement_policy == 1)
        {
            // FIFO
            // get index of first element in queue
            int fifo_index = sets[set_index].fifo_position.front();

            // If that line to be replaced is dirty, increment writeback
            if (sets[set_index].lines[fifo_index].dirty)
            {
                writebacks++;
            }

            // Perform tag replacement
            sets[set_index].lines[fifo_index].tag = tag;
            sets[set_index].lines[fifo_index].dirty = (op == 'w');

            // Move index from front of queue to the back
            update_fifo(set_index, fifo_index);
        }
        else if (replacement_policy == 2)
        {
            // OPTIMAL
            // Implement OPTIMAL policy specific logic here
        }
    }
    return true;
}

bool Cache::simulate_access(char op, long long address)
{
    writeback_flag = false;
    eviction_flag = false;

    int log_block_size = static_cast<int>(log2(block_size));
    int set_index = (address >> log_block_size) % num_sets;
    long long tag = address >> (log_block_size + static_cast<int>(log2(num_sets)));

    // Increment reads or writes count based on operation type
    if (op == 'r')
    {
        reads_count++;
    }
    else if (op == 'w')
    {
        writes_count++;
    }

    // Search for the tag in the set
    bool hit = false;
    bool wasDirty = false;
    for (int i = 0; i < assoc; i++)
    {
        if (sets[set_index].lines[i].tag == tag)
        {
            // Hit found
            hit = true;
            hit_count++;
            if (op == 'w')
            {
                sets[set_index].lines[i].dirty = true;
            }
            update_lru(set_index, i);
            break;
        }

        if (sets[set_index].lines[i].dirty)
        {
            wasDirty = true; // Set wasDirty if any block in the set is dirty
        }
    }

    if (!hit)
    {
        // Miss
        // miss_count++;
        // Both write misses and read misses will cause block to be allocated in Cache.
        allocate_block(set_index, tag, op);
        // allocate_block(set_index, tag, wasDirty);

        if (op == 'r')
        {
            read_misses++;
        }
        else if (op == 'w')
        {
            write_misses++;
        }
    }

    return hit;
}

void Cache::L1_print_statistics()
{
    // Updated to print additional required statistics
    unsigned long long accesses = reads_count + writes_count;
    float miss_rate = accesses > 0 ? static_cast<float>(read_misses + write_misses) / accesses : 0;

    cout << "Read operations: " << reads_count << "\n";
    cout << "Read Misses: " << read_misses << "\n";
    cout << "Write operations: " << writes_count << "\n";
    cout << "Write Misses: " << write_misses << "\n";
    cout << "Writebacks: " << writebacks << "\n";
    cout << "Miss Rate: " << fixed << setprecision(4) << (accesses > 0 ? miss_rate * 100 : 0) << "%\n";
    // cout << "Miss rate: " << static_cast<float>(miss_count) / (hit_count + miss_count) * 100 << "%\n";
}

void Cache::L2_print_statistics()
{
    // Updated to print additional required statistics
    unsigned long long accesses = reads_count + writes_count;
    float miss_rate = accesses > 0 ? static_cast<float>(read_misses + write_misses) / accesses : 0;

    cout << "Read operations: " << reads_count << "\n";
    cout << "Read Misses: " << read_misses << "\n";
    cout << "Write operations: " << writes_count << "\n";
    cout << "Write Misses: " << write_misses << "\n";
    cout << "Writebacks: " << writebacks << "\n";
    // cout << "Miss Rate: " << fixed << setprecision(4) << (accesses > 0 ? miss_rate * 100 : 0) << "%\n";
    // cout << "Miss rate: " << static_cast<float>(miss_count) / (hit_count + miss_count) * 100 << "%\n";
    cout << "Miss Rate: " << (static_cast<float>(read_misses) / (reads_count)) * 100 << "%\n";
}

void Cache::print_contents()
{
    cout << "Final Cache Contents:\n";
    for (unsigned long long i = 0; i < num_sets; ++i)
    {
        cout << "Set " << i << ":";
        for (auto &line : sets[i].lines)
        {
            if (line.tag != -1)
                cout << " [" << hex << line.tag << (line.dirty ? " D" : "") << "]";
            else
                cout << " [Empty]";
        }
        cout << dec << "\n"; // Switch back to decimal for non-hex output
    }
}

#endif // CACHE_H