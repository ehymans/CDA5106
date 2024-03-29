/*
Ethan Hymans
Daniel Solis
Kyle Shervington
<Name>
<Name>
CDA5106 - Advanced Computer Architecture
Machine Problem 1: Cache Design, Memory Hierarchy Design
*/

/*
TO-DO LIST:
  1) For debug5.txt, L2 output statistics are incorrect. The # of reads/writes is lower than what is expected. The expected results are at the end of debug5.txt.
  2) FIFO & Optimal Replacement Polciies
        - FIFO might involve using a queue of block index's
  3) Handling INCLUSIVE Cache. 
  4) Moving from L1 to L2 cache has some problems like evicting the block and passing it to L2
  5) Overall structure, checking the parameters before different actions are done on the cache (LRU, FIFO, evicting, replacing, etc) 

  ...
*/


#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <deque>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <unordered_map>

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
        for(int i = 0; i < assoc; ++i) 
        {
            lru_position[i] = i;
        }
    }

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
        // replace the block that would be used furthest into the future
    }
};


class Cache 
{
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

    //map<long long, vector<int>> next_use_index;
    deque<long long> accesses;

public:
    Cache(unsigned int size, unsigned int assoc, unsigned int block_size, unsigned int replacement, unsigned int inclusion) : 
        assoc(assoc), block_size(block_size), replacement_policy(replacement), inclusion_policy(inclusion) {
        num_sets = size == 0 ? 0 : size / (block_size * assoc); 
        sets.resize(num_sets, CacheSet(assoc));
    }

    // Add getter methods for required properties - 3/25/24
    unsigned long long getNumSets() const { return num_sets; }
    unsigned int getAssoc() const { return assoc; }
    unsigned int getBlockSize() const { return block_size; }

    // additional output stats - 3/25/24
    unsigned long long read_misses = 0;
    unsigned long long write_misses = 0;
    unsigned long long writebacks = 0;


    ////////////////////////////////////////////////
    ////////////// EVICT_BLOCK /////////////////////
    ////////////////////////////////////////////////   
    void evict_block(int set_index, int block_index) 
    {
        // Assuming non-inclusive cache, thus not checking L1
        if (sets[set_index].lines[block_index].dirty) 
        {
            // increment the write-back counter
            writebacks++;
        }
        // Reset the block
        sets[set_index].lines[block_index].tag = -1;
        sets[set_index].lines[block_index].dirty = false;
    }

    ////////////////////////////////////////////////
    ////////////// UPDATE_LRU //////////////////////
    ////////////////////////////////////////////////

    void update_lru(int set_index, int accessed_index) 
    {
        // Move the accessed block to the most recently used position
        auto accessed_lru = std::find(sets[set_index].lru_position.begin(), sets[set_index].lru_position.end(), accessed_index);
        if (accessed_lru != sets[set_index].lru_position.end()) 
        {
            sets[set_index].lru_position.erase(accessed_lru);
        }
        sets[set_index].lru_position.push_back(accessed_index);
    } 

    ////////////////////////////////////////////////
    ////////////// UPDATE_NEXT_USE /////////////////
    ////////////////////////////////////////////////
    void update_next_use(){
        // decrement next use by 1 for all blocks except newly inserted block
        // if 0 reset to next OR -1 indicated never used again
        // (because we can safely dec and check < 0 for checks)

        /*map<long long, vector<int>>::iterator ptr = next_use_index.begin();
        while (ptr != next_use_index.end())
        {
            vector<int>::iterator vecptr = ptr->second.begin();
            while (vecptr < ptr->second.end())
            {
                //(*vecptr) = (*vecptr) - 1;
                if ((*vecptr) <= 0){
                    vecptr = ptr->second.erase(vecptr);
                }
                else
                {
                    vecptr++;
                }

            }

            ptr++;
        }*/
        accesses.pop_front();
    }


    ////////////////////////////////////////////////
    ////////////// UPDATE_NEXT_USE /////////////////
    ////////////////////////////////////////////////
    void set_next_use(deque<long long> in_accesses){
        // decrement next use by 1 for all blocks except newly inserted block
        // if 0 reset to next OR -1 indicated never used again
        // (because we can safely dec and check < 0 for checks)

        // deep copy of map since the map contains vectors we need to do this manually
        /*for (map<long long, vector<int>>::iterator ptr = accesses.begin();
                ptr != accesses.end(); ptr++)
        {
            //deep copy
            vector<int> indexes(ptr->second);

            //new map elem with Key and new vector
            next_use_index.emplace(ptr->first, indexes);
        }*/

        accesses = in_accesses;
    }

    ////////////////////////////////////////////////
    ////////////// ALLOCATE_BLOCK //////////////////
    ////////////////////////////////////////////////
   
    void allocate_block(int set_index, long long tag, char op) 
    {
    bool foundEmptyLine = false;
        for (int i = 0; i < assoc; ++i) 
        {
            if (sets[set_index].lines[i].tag == -1) 
            { // Empty line found
                sets[set_index].lines[i].tag = tag;
                sets[set_index].lines[i].dirty = (op == 'w'); // Set dirty if it's a write
                update_lru(set_index, i); // Move to the most recently used position
                foundEmptyLine = true;
                break;
            }
        }

        if (!foundEmptyLine)
        {
            if(replacement_policy == 0)
            {
                // lru replacement policy here
                int lru_index = distance(sets[set_index].lru_position.begin(), min_element(sets[set_index].lru_position.begin(), sets[set_index].lru_position.end()));

                //evict_block(set_index, lru_index);
                
                if (sets[set_index].lines[lru_index].dirty) 
                {
                    writebacks++;
                }

                sets[set_index].lines[lru_index].tag = tag;
                sets[set_index].lines[lru_index].dirty = (op == 'w'); // Set dirty based on operation
                update_lru(set_index, lru_index);
            }
            else if(replacement_policy == 1)
            {
                // FIFO
            }
            else if(replacement_policy == 2)
            {
                // Optimal
                int optimal_index = -1;
                int highestFutureUse = -1;
                for (int i = 0; i < assoc; i++){
                    /*vector<int> nextUse = next_use_index[sets[set_index].lines[i].tag];
                    if (nextUse.empty()){
                        optimal_index = i;
                        break;
                    }
                    int j = 0;
                    while (nextUse[j] < 1 && j < nextUse.size()) {
                        j++;
                    }
                    if (j >= nextUse.size()){
                        optimal_index = i;
                        break;
                    }
                    if (nextUse[j] > highestFutureUse) {
                        highestFutureUse = nextUse[j];
                        optimal_index = i;
                    }*/
                    bool found = false;
                    for (int j = 0; j < accesses.size(); j++){
                        if (accesses[j] == sets[set_index].lines[i].tag)
                        {
                            if (j > highestFutureUse)
                            {
                                highestFutureUse = j;
                                optimal_index = i;
                            }
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        optimal_index = i;
                        break;
                    }
                }

                if (optimal_index == -1){
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
    }

    ////////////////////////////////////////////////
    ////////////// SIMULATE_ACCESS /////////////////
    ////////////////////////////////////////////////
    bool simulate_access(char op, long long address) 
    {
        int log_block_size = static_cast<int>(log2(block_size));
        int set_index = (address >> log_block_size) % num_sets;
        long long tag = address >> (log_block_size + static_cast<int>(log2(num_sets)));

        // Increment reads or writes count based on operation type
        if (op == 'r') 
        {
            reads_count++;
            //cout << "Read counter Incremented: " << reads_count << "\n";
        } 
        else if (op == 'w') 
        {
            writes_count++;
        }

        // Search for the tag in the set
        bool hit = false;
        for (int i = 0; i < assoc; i++) {
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
        }

        if (!hit) 
        {
            // Miss
            miss_count++;
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

    void print_statistics() 
    {
        // Updated to print additional required statistics
        unsigned long long accesses = reads_count + writes_count;
        float miss_rate = accesses > 0 ? static_cast<float>(read_misses + write_misses) / accesses : 0;
        
        cout << "Read operations: " << reads_count << "\n";
        cout << "Read Misses: " << read_misses << "\n";
        cout << "Write operations: " << writes_count << "\n";
        cout << "Write Misses: " << write_misses << "\n";
        cout << "Writebacks: " << writebacks << "\n";
        cout << "Miss Rate: " << fixed << setprecision(2) << (accesses > 0 ? miss_rate * 100 : 0) << "%\n";
        //cout << "Miss rate: " << static_cast<float>(miss_count) / (hit_count + miss_count) * 100 << "%\n";
    }


    // New method to output the final contents of the cache - 3/25/24
    void print_contents() 
    {
        cout << "Final Cache Contents:\n";
        for (unsigned long long i = 0; i < num_sets; ++i) {
            cout << "Set " << i << ":";
            for (auto& line : sets[i].lines) 
            {
                if (line.tag != -1)
                    cout << " [" << hex << line.tag << (line.dirty ? " D" : "") << "]";
                else
                    cout << " [Empty]";
            }
            cout << dec << "\n"; // Switch back to decimal for non-hex output
        }
    }

};

struct  Operation {
    int op;
    long long address;
    Operation (int in_op, long long in_address){
        op = in_op;
        address = in_address;
    }
};

class Simulation 
{
private:
    Cache L1_cache;
    Cache L2_cache; 
    string trace_file;
    unsigned int replacement_policy;
    bool isL2Enabled; // Added flag to indicate if L2 is enabled
public:
    // constructor to initialize both L1 and L2 caches
    Simulation(unsigned int block_size, unsigned int L1_size, unsigned int L1_assoc, unsigned int L2_size, unsigned int L2_assoc, unsigned int replacement, unsigned int inclusion, const string& trace_file) :
        L1_cache(L1_size, L1_assoc, block_size, replacement, inclusion), 
        L2_cache(L2_size, L2_assoc, block_size, replacement, inclusion), // Pass block_size here
        trace_file(trace_file),
        replacement_policy(replacement),
        isL2Enabled(L2_size != 0 && L2_assoc != 0) {} // Initialize L2 enabled status based on size and assoc

    void run() 
    {
        cout << "Memory Hierarchy Configuration and Trace Filename:\n";
        cout << "L1 Cache: " << L1_cache.getNumSets() * L1_cache.getAssoc() * L1_cache.getBlockSize() / 1024 
             << "KB " << L1_cache.getAssoc() << "-way, Block size: " << L1_cache.getBlockSize() << "B\n";

        // Conditionally display L2 cache configuration based on whether it's enabled
        
        if (isL2Enabled) 
        {
            cout << "L2 Cache: " << L2_cache.getNumSets() * L2_cache.getAssoc() * L2_cache.getBlockSize() / 1024 
                 << "KB " << L2_cache.getAssoc() << "-way, Block size: " << L2_cache.getBlockSize() << "B\n";
        } 
        else 
        {
            cout << "L2 Cache: Disabled\n";
        }

        cout << "Trace file: " << trace_file << "\n";

        ifstream inp(trace_file);
        if (!inp) 
        {
            cerr << "Error opening trace file\n";
            return;
        }

        char op;
        long long address;
        //map<long long, vector<int>> accesses;
        deque<long long> accesses;

        //int count = 0;
        vector<Operation> lines;
        // preprocessing step required for optimal replacement
        while (inp >> op >> hex >> address)
        {
            Operation line(op,address);

            accesses.push_back(address);

            /*map<long long, vector<int>>::iterator elem = accesses.find(address);

            // check if elem was found
            if(elem != accesses.end())
            {
                elem->second.push_back(count);
            }
            else
            {
                vector<int> v({count});
                accesses.emplace(address, v);
            }*/

            lines.push_back(line);
            //count++;
        }

        L1_cache.set_next_use(accesses);
        L2_cache.set_next_use(accesses);

        for (vector<Operation>::iterator itr = lines.begin();
                itr < lines.end(); itr++)
        {
            //cout << "Operation: " << op << ", Address: " << address << "\n"; // Debug print
            // First, attempt access in L1 cache
            if (!L1_cache.simulate_access(itr->op, itr->address) && isL2Enabled)
            {
                // If miss in L1, access L2 cache
                L2_cache.simulate_access(itr->op, itr->address);
            }

            if (replacement_policy == 2)
            {
                L1_cache.update_next_use();
                if (isL2Enabled)
                {
                    L2_cache.update_next_use();
                }
            }
        }

        inp.close();

        cout << "L1 Cache Contents:\n";
        L1_cache.print_contents();

        if (isL2Enabled) {
            cout << "L2 Cache Contents:\n";
            L2_cache.print_contents();
        }

        cout << "\nL1 Cache Statistics\n";
        L1_cache.print_statistics();    // L1 stats

        if (isL2Enabled) {
            cout << "\nL2 Cache Statistics\n";
            L2_cache.print_statistics();    // L2 stats
        }
    }
};

/*////////////////////////////////////////////////
********************* MAIN **********************
///////////////////////////////////////////////*/
int main(int argc, char* argv[]) 
{
    /* 8KB 4-way set-associative L1 cache with 32B block size, 256KB 8-way set-associative L2
    * cache with 32B block size, LRU replacement, non-inclusive cache (default), gcc trace:
    * 
    * ./sim_cache 32 8192 4 262144 8 0 0 traces/gcc_trace.txt
    * 
    * debug5.txt comparision testing: ./sim_cache 16 1024 1 8192 4 0 0 traces/go_trace.txt
    */

    if (argc != 9) 
    {
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
