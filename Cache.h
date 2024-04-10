#ifndef CACHE_H
#define CACHE_H

// includes
#include "CacheComponents.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <map>
#include <climits>

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
    unsigned long long total_memory_traffic = 0;

    // additional output stats
    unsigned long long read_misses = 0;
    unsigned long long write_misses = 0;
    unsigned long long writebacks = 0;
    unsigned long long inclusive_writeback_counter = 0;
    // bool writeback_flag = false;

    // @optimal
    map<long long, queue<int>> *next_use_index;

public:
    Cache(unsigned int size, unsigned int assoc, unsigned int block_size, unsigned int replacement, unsigned int inclusion) : assoc(assoc), block_size(block_size), replacement_policy(replacement), inclusion_policy(inclusion)
    {
        num_sets = size == 0 ? 0 : size / (block_size * assoc);
        sets.resize(num_sets, CacheSet(assoc));
    }

    float gMissRate = 0;

    // Public methods for Cache operations
    unsigned long long getNumSets() const { return num_sets; }
    unsigned int getAssoc() const { return assoc; }
    unsigned int getBlockSize() const { return block_size; }
    // Graph #2
    long long getReadCount() {return reads_count;};
    long long getWriteCount() {return writes_count;};
    long long getReadMissCount() {return read_misses;};
    long long getWriteMissCount() {return write_misses;};

    bool evict_block(int set_index, int block_index);

    void update_lru(int set_index, int accessed_index);
    void update_fifo(int set_index, int index);

    bool allocate_block(int set_index, long long tag, char op);

    bool simulate_access(char op, long long address);

    bool check_and_invalidate(long long address);

    void L1_print_statistics();
    void L2_print_statistics();

    void print_contents();
    int calculate_set_index(long long address);
    long long calculate_tag(long long address);
    long long calculate_address(long long tag, int set_index) const;

    int find_lru_block(int set_index);
    bool writeback_flag;
    bool eviction_flag;

    long long evicted_address;

    void handle_writeback(long long address);
    bool l2_miss_on_l1_eviction(char op, long long address);
    bool hit_miss_simulate(char op, long long address);

    bool L2_simulate_access(char op, long long address);
    void calculate_memory_traffic();
    int calculate_inclusive_memory_traffic();
    int return_inclusive_writeback_counter();

    // @optimal
    void set_next_use(map<long long, queue<int>> &in_accesses);
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

long long Cache::calculate_address(long long tag, int set_index) const
{
    return (tag << (static_cast<int>(log2(block_size)) + static_cast<int>(log2(num_sets)))) | (set_index << static_cast<int>(log2(block_size)));
}

int Cache::find_lru_block(int set_index)
{
    // first element is LRU block
    if (!sets[set_index].lru_position.empty())
    {
        return sets[set_index].lru_position.front();
    }
    else
    {
        // when there isn't any blocks
        return -1; // return invalid index
    }
}

// @optimal
void Cache::set_next_use(map<long long, queue<int>> &accesses)
{
    // just a pointer to the map in Simulator to avoid duplicating data
    next_use_index = &accesses;
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

            // Since we just used this block, update its LRU position
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
            int optimal_index = -1;
            int highestFutureUse = -1;
            for (int i = 0; i < assoc; i++)
            {
                long long block_address = calculate_address(sets[set_index].lines[i].tag, set_index);

                // iterate through all possible addresses maping to this block for their next use
                // we only care about the NEXT use
                int next_use_of_block = INT_MAX;
                for (int j = 0; j < block_size; j++)
                {
                    auto ptr = next_use_index->find(block_address + j);
                    if (ptr == next_use_index->end())
                    {
                        continue;
                    }

                    if (ptr->second.empty())
                    {
                        continue;
                    }

                    if (ptr->second.front() < next_use_of_block)
                    {
                        next_use_of_block = ptr->second.front();
                    }
                }

                if (next_use_of_block == INT_MAX)
                {
                    optimal_index = i;
                    break;
                }

                if (next_use_of_block > highestFutureUse)
                {
                    highestFutureUse = next_use_of_block;
                    optimal_index = i;
                }
            }

            if (optimal_index == -1)
            {
                // problem
                optimal_index = 0;
            }

            if (sets[set_index].lines[optimal_index].dirty)
            {
                writebacks++;
            }

            sets[set_index].lines[optimal_index].tag = tag;
            sets[set_index].lines[optimal_index].dirty = (op == 'w'); // Set dirty based on operation
        }
    }
    return true;
}

// for inclusive cache --> check if the block is there and invalidate
bool Cache::check_and_invalidate(long long address)
{
    int log_block_size = static_cast<int>(log2(block_size));
    int set_index = (address >> log_block_size) % num_sets;
    long long tag = address >> (log_block_size + static_cast<int>(log2(num_sets)));

    // iterate through the set to find a matching tag
    for (int i = 0; i < assoc; i++)
    {
        if (sets[set_index].lines[i].tag == tag)
        {
            // Block found, invalidate it
            bool wasDirty = sets[set_index].lines[i].dirty;
            sets[set_index].lines[i].tag = -1;      // invalidate the block
            sets[set_index].lines[i].dirty = false; // clear the dirty flag

            // If the block was dirty --> writeback to main memory
            if (wasDirty)
            {
                inclusive_writeback_counter++;
            }

            return wasDirty; // return true if the block was dirty
        }
    }

    // Block not found or not dirty, no writeback needed
    return false;
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
        // Both write misses and read misses will cause block to be allocated in Cache.
        allocate_block(set_index, tag, op);

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

void Cache::calculate_memory_traffic()
{
    total_memory_traffic = (read_misses + write_misses + writebacks);
    cout << "Total Memory Traffic: " << total_memory_traffic << "\n";
}

int Cache::calculate_inclusive_memory_traffic()
{
    total_memory_traffic = (read_misses + write_misses + writebacks);
    return total_memory_traffic;
}

int Cache::return_inclusive_writeback_counter()
{
    return inclusive_writeback_counter;
}

void Cache::L1_print_statistics()
{
    // Updated to print additional required statistics
    unsigned long long accesses = reads_count + writes_count;
    float miss_rate = accesses > 0 ? static_cast<float>(read_misses + write_misses) / accesses : 0;

    gMissRate = miss_rate * 100;

    cout << "Read operations: " << reads_count << "\n";
    cout << "Read Misses: " << read_misses << "\n";
    cout << "Write operations: " << writes_count << "\n";
    cout << "Write Misses: " << write_misses << "\n";
    cout << "Writebacks: " << writebacks << "\n";
    cout << "Miss Rate: " << fixed << setprecision(4) << (accesses > 0 ? miss_rate * 100 : 0) << "%\n";
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
    cout << "Miss Rate: " << (static_cast<float>(read_misses) / (reads_count)) * 100 << "%\n"; // this MR calculation is specific to L2.
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