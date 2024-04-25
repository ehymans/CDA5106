#ifndef SIMULATION_H
#define SIMULATION_H

#include "Cache.h"
#include <string>
#include <fstream>
#include <iostream>
#include <map>

class Simulation {
private:
    Cache L1_cache;
    Cache L2_cache;
    std::string trace_file;
    bool isL2Enabled;
    unsigned int inclusionPolicy;
    unsigned int total_memory_traffic;

    // optimal replacement 
    unsigned int replacement_policy;
    map<long long, queue<int>> accesses;

public:
    Simulation(unsigned int block_size, unsigned int L1_size, unsigned int L1_assoc, 
               unsigned int L2_size, unsigned int L2_assoc, 
               unsigned int replacement, unsigned int inclusion, 
               const std::string& trace_file)
        : L1_cache(L1_size, L1_assoc, block_size, replacement, inclusion),
          L2_cache(L2_size, L2_assoc, block_size, replacement, inclusion),
          trace_file(trace_file),
          replacement_policy(replacement),
          isL2Enabled(L2_size != 0 && L2_assoc != 0), inclusionPolicy(inclusion) {}

    void run();
};

void Simulation::run() 
{
    std::cout << "===== Simulator configuration =====\n";
    std::cout << "BLOCKSIZE: " << L1_cache.getBlockSize() << "\n";
    std::cout << "L1_SIZE: " << L1_cache.getNumSets() * L1_cache.getAssoc() * L1_cache.getBlockSize() << "\n";
    std::cout << "L1_ASSOC: " << L1_cache.getAssoc() << "\n";

    if (isL2Enabled)
    {
        std::cout << "L2_SIZE: " << L2_cache.getNumSets() * L2_cache.getAssoc() * L2_cache.getBlockSize() << "\n";
        std::cout << "L2_ASSOC: " << L2_cache.getAssoc() << "\n";
    }
    else
    {
        std::cout << "L2_SIZE: " << 0 << "\n";
        std::cout << "L2_ASSOC: " << 0 << "\n";
    }

    // Print appropriate replacement policy
    if (replacement_policy == 0)
    {
        std::cout << "REPLACEMENT POLICY: LRU\n";
    }
    else if (replacement_policy == 1)
    {
        std::cout << "REPLACEMENT POLICY: FIFO\n";
    }
    else if (replacement_policy == 2)
    {
        std::cout << "REPLACEMENT POLICY: OPTIMAL\n";
    }
    else
    {
        std::cerr << "Invalid replacement policy\n";
        return;
    }

    std::cout << "INCLUSION PROPERTY: " << (inclusionPolicy == 0 ? "non-inclusive" : "inclusive") << "\n";

    // Remove "traces/" from the start of the trace file name
    string trace_file_name = trace_file.substr(7);

    std::cout << "trace_file: " << trace_file_name << "\n";
    std::ifstream inp(trace_file);
    if (!inp) {
        std::cerr << "Error opening trace file\n";
        return;
    }

    char op;
    long long address;
    int l2_writeback_counter = 0;

    // for optimal only //
    long long previous;
    int count = 0;
    /////////////////////
    int L1_writeback_from_invalidation_counter = 0;         // test counter 

    //////////// OPTIMAL PRE-PROCESSING ////////////////
    while (inp >> op >> hex >> address) 
    {
        // @optimal
        // setup map of next usages
        if (previous != address) {
            //add previous
            map<long long, queue<int>>::iterator elem = accesses.find(previous);
            // check if elem was found
            if (elem != accesses.end()) {
                elem->second.push(count - 1);
            } else {
                queue<int> v({count - 1});
                accesses.emplace(previous, v);
            }
        }

        count++;
        previous = address;
    } 

    // need to insert the last item
    map<long long, queue<int>>::iterator elem = accesses.find(previous);
    // check if elem was found
    if(elem != accesses.end())
    {
        elem->second.push(count-1);
    }
    else
    {
        queue<int> v({count-1});
        accesses.emplace(previous, v);
    }


    L1_cache.set_next_use(accesses);
    if (isL2Enabled)
    {
        L2_cache.set_next_use(accesses);
    }

    inp.close();
    //////////////// END OF OPTIMAL PRE-PROCESSING /////////////////////////

    inp.open(trace_file);
    int current_line = 0;

    while (inp >> op >> hex >> address)
    {

        if(inclusionPolicy == 0)    // for non-inclusive cache
        {
            bool hitInL1 = L1_cache.simulate_access(op, address); // returns hit (true) or miss (false)

            if ((L1_cache.writeback_flag) && isL2Enabled)
            {
                // L2 writes: equal to the number of dirty blocks evicted from L1 that need to be written back to L2 or main memory.
                bool isL2_writeback_hit = L2_cache.simulate_access('w', L1_cache.evicted_address);
            }

            if (!hitInL1 && isL2Enabled) // if miss in L1 and L2 is enabled
            {
                bool hitInL2 = L2_cache.simulate_access('r', address); // read L2 cache and attempt to find address
            }

            // @optimal
            if (replacement_policy == 2)
            {
                if (accesses[address].front() <= current_line)
                {
                    accesses[address].pop();
                }

            }
            current_line++;

        }
        else if(inclusionPolicy == 1) // for inclusive cache
        {
            bool hitInL1 = L1_cache.simulate_access(op, address); // Returns hit (true) or miss (false)

            if ((L1_cache.writeback_flag) && isL2Enabled)
            {
                // L2 writebacks: equal to the number of dirty blocks evicted from L1 that need to be written back to L2 or main memory.
                bool isL2_writeback_hit = L2_cache.simulate_access('w', L1_cache.evicted_address);
                // In case of a miss in L2, we must handle it according to the inclusive policy, including potential evictions.
                if (!isL2_writeback_hit)
                {
                    // If evicting a block from L2, invalidate the corresponding block in L1 if it exists
                    // The check_and_invalidate method should return true if the evicted block was dirty --> signals a direct WB to main mem.
                    if(L2_cache.eviction_flag)
                    {
                        bool L1_dirty_block_needs_writeback = L1_cache.check_and_invalidate(L2_cache.evicted_address);
                        if(L1_dirty_block_needs_writeback)
                        {
                            // this would be a direct writeback to main mem.
                        }
                    }
                }
            }

            if (!hitInL1 && isL2Enabled) // If miss in L1 and L2 is enabled
            {
                bool hitInL2 = L2_cache.simulate_access('r', address); // Read L2 cache and attempt to find address
                if (!hitInL2)
                {
                    // Upon a miss in L2, when allocating a new block in L2, make sure to also check L1 for inclusivity
                    // If a block is evicted from L2, check if it exists in L1 and invalidate it
                    if(L2_cache.eviction_flag)
                    {
                        bool L1_dirty_block_needs_writeback = L1_cache.check_and_invalidate(L2_cache.evicted_address);
                        if(L1_dirty_block_needs_writeback)
                        {
                            // If the invalidated block in L1 was dirty, handle direct writeback to main memory here.
                        }
                    }
                }
            }
        }
    }

    inp.close();   
      
     cout << "===== L1 contents =====\n";
     L1_cache.print_contents();


    // Final output and statistics
    if (isL2Enabled) 
    {
        cout << "===== L2 contents =====\n";
        L2_cache.print_contents();
    }

    cout << "\n===== Simulation results (raw) =====\n";
    L1_cache.L1_print_statistics();    // L1 stats

    if (isL2Enabled) 
    {
        L2_cache.L2_print_statistics();    // L2 stats
    }
    else
    {
        // Print zero values for L2 statistics when L2 is not enabled
        cout << "g. number of L2 reads: 0\n";
        cout << "h. number of L2 read misses: 0\n";
        cout << "i. number of L2 writes: 0\n";
        cout << "j. number of L2 write misses: 0\n";
        cout << "l. number of L2 writebacks: 0\n";
        cout << "k. L2 miss rate: 0\n";
    }

    //////////// MEMORY TRAFFIC CALCULATION ////////////
    if(inclusionPolicy == 0 && isL2Enabled)
    {
        // non-inclusive, L2 enabled
        L2_cache.calculate_memory_traffic();
    }
    else if(inclusionPolicy == 1 && isL2Enabled)
    {
        // inclusive, L2 enabled
        total_memory_traffic = (L2_cache.calculate_inclusive_memory_traffic() + L1_cache.return_inclusive_writeback_counter());
        cout << "m. total memory traffic: " << total_memory_traffic << "\n";
    }
    else if(inclusionPolicy == 0 && (!isL2Enabled))
    {
        L1_cache.calculate_memory_traffic();
    }
    else if(inclusionPolicy == 1 && (!isL2Enabled))
    {
        L1_cache.calculate_memory_traffic();
    }
    //////////////////////////////////////////////////////

}

#endif // SIMULATION_H