#include "Simulation.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 9)
    {
        std::cerr << "Usage: " << argv[0] << " <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_POLICY> <TRACE_FILE>\n";
        return 1;
    }

    try
    {
        unsigned int block_size = std::stoi(argv[1]);
        unsigned int L1_size = std::stoi(argv[2]);
        unsigned int L1_assoc = std::stoi(argv[3]);
        unsigned int L2_size = std::stoi(argv[4]);
        unsigned int L2_assoc = std::stoi(argv[5]);
        unsigned int replacement_policy = std::stoi(argv[6]);
        unsigned int inclusion_policy = std::stoi(argv[7]);
        std::string trace_file = argv[8];

        Simulation sim(block_size, L1_size, L1_assoc, L2_size, L2_assoc, replacement_policy, inclusion_policy, trace_file);
        sim.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Err parsing command-line arguments: " << e.what() << '\n';
        return 2;
    }

    return 0;
}

/* --- VALIDATION CASES ---
0. .\sim_cache 16 1024 2 0 0 0 0 traces/gcc_trace.txt           PASS
1. .\sim_cache 16 1024 1 0 0 0 0 traces/perl_trace.txt          PASS
2. .\sim_cache 16 1024 2 0 0 1 0 traces/gcc_trace.txt           PASS
3. .\sim_cache 16 1024 2 0 0 2 0 traces/vortex_trace.txt        PASS
4. .\sim_cache 16 1024 2 8192 4 0 0 traces/gcc_trace.txt        PASS
5. .\sim_cache 16 1024 1 8192 4 0 0 traces/go_trace.txt         PASS
6. .\sim_cache 16 1024 2 8192 4 0 1 traces/gcc_trace.txt        PASS
7. .\sim_cache 16 1024 1 8192 4 0 1 traces/compress_trace.txt   PASS
*/
