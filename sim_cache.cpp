/*
Ethan Hymans
Daniel Solis
Kyle Shervington
<Name>
<Name>
CDA5106 - Advanced Computer Architecture
Machine Problem 1: Cache Design, Memory Hierarchy Design
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

using namespace std;

class CacheLine 
{
public:
    long long tag;
    bool dirty;
    CacheLine() : tag(-1), dirty(false) {}
};

class CacheSet 
{
public:
    vector<CacheLine> lines;
    vector<long long> lru_position;
    vector<int> fifo_position;          // this may need to be changed from an int

    CacheSet(int assoc) : lines(assoc), lru_position(assoc), fifo_position(assoc, 0) 
    {
        for(int i = 0; i < assoc; ++i) {
            lru_position[i] = i;
        }
    }

    void replace_block_lru(long long tag);
    void replace_block_fifo(long long tag);
    void replace_block_optimal(long long tag, const vector<long long>& future_references);


    // methods will need to be adapted to fit our paper implementation
    void replace_block_lru(long long tag) 
    {
        // LRU implementation: Find least recently used block and replace it with new tag
        int lru_index = distance(lru_position.begin(), min_element(lru_position.begin(), lru_position.end()));
        lines[lru_index].tag = tag;
        // need to include dirty bit handling
    }

    void replace_block_fifo(long long tag) 
    {
        // FIFO implementation: Replace the oldest block
        int fifo_index = fifo_position.front();
        fifo_position.erase(fifo_position.begin());     // Remove the oldest block's index
        fifo_position.push_back(fifo_index);            // Add it back to signify it's the newest
        lines[fifo_index].tag = tag;
        // need to include dirty bit handling
    }

    void replace_block_optimal(long long tag, const vector<long long>& future_references) 
    {
        // gotta figure this one out 
    }
};

class Cache {
private:
    vector<CacheSet> sets;
    unsigned long long num_sets;
    unsigned int assoc;
    unsigned int block_size;
    unsigned int replacement_policy;
    unsigned int inclusion_policy;

    unsigned long long hit_count = 0;
    unsigned long long miss_count = 0;
    unsigned long long reads_count = 0;
    unsigned long long writes_count = 0;

public:
    Cache(unsigned int size, unsigned int assoc, unsigned int block_size, unsigned int replacement, unsigned int inclusion) : 
        assoc(assoc), block_size(block_size), replacement_policy(replacement), inclusion_policy(inclusion) {
        num_sets = size / (block_size * assoc); // Use block_size here
        sets.resize(num_sets, CacheSet(assoc));
    }

    //void simulate_access(char op, long long address);

    bool simulate_access(char op, long long address);
    void update_lru(int set, int index);
    void update_fifo(int set, int index);
    void allocate_block(int set_index, long long tag);
    void evict_block(int set_index, int block_index);
    
    void print_statistics() 
    {
        cout << "Miss rate: " << static_cast<float>(miss_count) / (hit_count + miss_count) * 100 << "%\n";
        cout << "Write operations: " << writes_count << "\n";
        cout << "Read operations: " << reads_count << "\n";
    }

    /*
    void simulate_access(char op, long long address) 
    {
        // Determine set and tag from address
        // Check for hit or miss
        // On write miss or read miss, allocate block (considering WBWA)        
        // Update state (LRU/FIFO/Optimal counters, valid and dirty bits)
    }*/

    void allocate_block(int set_index, long long tag) 
    {
        // Check if there is space, if not find a victim block to evict based on replacement policy
        // If victim block is dirty, issue a writeback
        // Bring in the requested block
    }

    void evict_block(int set_index, int block_index) 
    {
        // If inclusive, invalidate corresponding block in L1 (if exists)
        // Issue writeback if dirty
    }
};

bool Cache::simulate_access(char op, long long address) 
{
    // this is going to need to be adjusted based on the implementation that we do
    int log_block_size = static_cast<int>(log2(block_size));
    int set_index = (address >> log_block_size) % num_sets;
    long long tag = address >> (log_block_size + static_cast<int>(log2(num_sets)));

    // Search for the tag in the set
    for (int i = 0; i < assoc; i++) {
        if (sets[set_index].lines[i].tag == tag) {
            // Hit
            if (op == 'W') {
                sets[set_index].lines[i].dirty = true;
            }
            update_lru(set_index, i); // Update LRU if using LRU policy
            return true; // Hit
        }
    }

    // Miss: Allocate block
    allocate_block(set_index, tag);
    return false; // Miss
}

class Simulation 
{
private:
    Cache L1_cache;
    Cache L2_cache; 
    string trace_file;
public:
    // constructor to initialize both L1 and L2 caches
    Simulation(unsigned int block_size, unsigned int L1_size, unsigned int L1_assoc, unsigned int L2_size, unsigned int L2_assoc, unsigned int replacement, unsigned int inclusion, const string& trace_file) :
        L1_cache(L1_size, L1_assoc, block_size, replacement, inclusion), 
        L2_cache(L2_size, L2_assoc, block_size, replacement, inclusion), // Pass block_size here
        trace_file(trace_file) {}

    void run() {
        ifstream inp(trace_file);
        if (!inp) {
            cerr << "Error opening trace file\n";
            return;
        }

        char op;
        long long address;
        while (inp >> op >> hex >> address) {
            // First, attempt access in L1 cache
            if (!L1_cache.simulate_access(op, address)) 
            {
                // If miss in L1, access L2 cache
                L2_cache.simulate_access(op, address);
            }
        }

        inp.close();
        L1_cache.print_statistics();    // L1 stats
        L2_cache.print_statistics();    // L2 stats
    }
};


/*////////////////////////////////////////////////
********************* MAIN **********************
///////////////////////////////////////////////*/
int main(int argc, char* argv[]) 
{
    if (argc != 8) {
        cerr << "Usage: <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_POLICY> <TRACE_FILE>\n";
        return 1;
    }

    // indexing and parsing
    unsigned int block_size = stoi(argv[1]);
    unsigned int L1_size = stoi(argv[2]);
    unsigned int L1_assoc = stoi(argv[3]);
    unsigned int L2_size = stoi(argv[4]);
    unsigned int L2_assoc = stoi(argv[5]);
    unsigned int replacement_policy = stoi(argv[6]);
    unsigned int inclusion_policy = stoi(argv[7]);
    string trace_file = argv[8];

    
    Simulation sim(block_size, L1_size, L1_assoc, L2_size, L2_assoc, replacement_policy, inclusion_policy, trace_file);
    sim.run();

    return 0;
}
