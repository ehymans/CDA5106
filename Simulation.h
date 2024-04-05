#ifndef SIMULATION_H
#define SIMULATION_H

#include "Cache.h"
#include <string>
#include <fstream>
#include <iostream>

class Simulation
{
private:
    Cache L1_cache;
    Cache L2_cache;
    std::string trace_file;
    bool isL2Enabled;

    // @optimal
    unsigned int replacement_policy;
    map<long long, queue<int>> accesses;
    /*struct Operation
    {
        int op;
        long long address;
        Operation(int in_op, long long in_address)
        {
            op = in_op;
            address = in_address;
        }
    };*/

public:
    Simulation(unsigned int block_size, unsigned int L1_size, unsigned int L1_assoc,
               unsigned int L2_size, unsigned int L2_assoc,
               unsigned int replacement, unsigned int inclusion,
               const std::string &trace_file)
        : L1_cache(L1_size, L1_assoc, block_size, replacement, inclusion),
          L2_cache(L2_size, L2_assoc, block_size, replacement, inclusion),
          trace_file(trace_file),
          replacement_policy(replacement), // @optimal
          isL2Enabled(L2_size != 0 && L2_assoc != 0)
    {
    }

    void run();
};

void Simulation::run()
{
    std::cout << "Memory Hierarchy Configuration and Trace Filename:\n";
    // Configuration output
    cout << "L1 Cache: " << L1_cache.getNumSets() * L1_cache.getAssoc() * L1_cache.getBlockSize() / 1024
         << "KB " << L1_cache.getAssoc() << "-way, Block size: " << L1_cache.getBlockSize() << "B\n";

    // Conditionally display L2 cache configuration based on whether it's enabled

    if (isL2Enabled)
    {
        cout << "L2 Cache: " << L2_cache.getNumSets() * L2_cache.getAssoc() * L2_cache.getBlockSize() / 1024
             << "KB " << L2_cache.getAssoc() << "-way, Block size: " << L2_cache.getBlockSize() << "B\n";
    }

    std::ifstream inp(trace_file);
    if (!inp)
    {
        std::cerr << "Error opening trace file\n";
        return;
    }

    char op;
    long long address;
    int l2_writeback_counter = 0;

    // @optimal
    //deque<long long> accesses;
    //vector<Operation> lines;
    long long previous;
    int count = 0;

    //pre-processing
    while (inp >> op >> hex >> address) {

        // @optimal
        //Operation line(op, address);
        //accesses.push_back(address);
        //lines.push_back(line);
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

    inp.open(trace_file);
    int current_line = 0;

    while (inp >> op >> hex >> address) {

        bool hitInL1 = L1_cache.simulate_access(op, address); // returns hit (true) or miss (false)
        /*if(hitInL1)
        {
            break;
        }*/

        if (!hitInL1 && isL2Enabled) // if miss in L1 and L2 is enabled
        {
            bool hitInL2 = L2_cache.simulate_access('r', address); // read L2 cache and attempt to find address
            // bool hitInL2 = L2_cache.hit_miss_simulate('r', address);
            // int L2_set_index = L2_cache.calculate_set_index(address);
            // int L2_tag = L2_cache.calculate_tag(address);

            /*
            if(!hitInL2)        // if it is a miss in L2
            {
                l2_writeback_counter++;
            }*/
        }

        if ((L1_cache.writeback_flag) && isL2Enabled)
        {
            // L2 writes: equal to the number of dirty blocks evicted from L1 that need to be written back to L2 or main memory.
            bool isL2_writeback_hit = L2_cache.simulate_access('w', L1_cache.evicted_address);

            /*
            if(!isL2_writeback_hit)     // if the L2 writeback itself is a MISS
            {

            }*/
        }

        // @optimal
        if (replacement_policy == 2)
        {
            /*L1_cache.update_next_use();
            if (isL2Enabled)
            {
                L2_cache.update_next_use();
            }*/

            if (accesses[address].front() <= current_line)
            {
                accesses[address].pop();
            }

        }
        current_line++;
    }

    inp.close();

    // @optimal
    /*L1_cache.set_next_use(accesses);
    L2_cache.set_next_use(accesses);*/

   /* for (vector<Operation>::iterator itr = lines.begin();
         itr < lines.end(); itr++)
    {
        // cout << "Operation: " << op << ", Address: " << address << "\n"; // Debug print
        //  First, attempt access in L1 cache
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
    }*/

    cout << "L1 Cache Contents:\n";
    L1_cache.print_contents();

    // Final output and statistics
    if (isL2Enabled)
    {
        cout << "L2 Cache Contents:\n";
        L2_cache.print_contents();
    }

    cout << "\nL1 Cache Statistics\n";
    L1_cache.L1_print_statistics(); // L1 stats

    if (isL2Enabled)
    {
        cout << "\nL2 Cache Statistics\n";
        L2_cache.L2_print_statistics(); // L2 stats
    }
}

#endif // SIMULATION_H