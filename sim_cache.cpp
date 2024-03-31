#include "Simulation.h"
#include <iostream>
#include <cstdlib> 

int main(int argc, char* argv[]) 
{
    if (argc != 9) {
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
    catch (const std::exception& e) {
        std::cerr << "Err parsing command-line arguments: " << e.what() << '\n';
        return 2;
    }

    return 0;
}
