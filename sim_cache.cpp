#include "Simulation.h"
#include <iostream>
#include <cstdlib>
#include <string>

const int BLOCK_SIZE = 32;
const int MISS_PENALTY = 100; // 100ns for block size of 32

string getGraph1Data(int (&assocArray)[5], int (&sizeArray)[11])
{
    string data = "";

    for (int assoc : assocArray)
    {
        data.append(to_string(assoc).append("-way Associativity: (size,miss-rate)\n"));

        for (int size : sizeArray)
        {
            Simulation sim(BLOCK_SIZE, size, assoc, 0, 0, 1, 0, "traces/gcc_trace.txt");
            sim.run();

            data.append(to_string((int)log2(size)).append(",").append(to_string(sim.getL1MissRate())).append("\n"));
        }

        data.append("\n");
    }

    return data;
}

string getGraph2Data(int (&assocArray)[5], int (&sizeArray)[11], double (&accessTimeArray)[5][11])
{
    string dataArray[12][6] = {{"Log2(SIZE)", "Direct Mapped", "2-Way", "4-Way", "8-Way", "Fully Associative"}};
    string data = "";

    for (int i = 0; i < (sizeof(sizeArray) / sizeof(int)); i++)
    {
        dataArray[i + 1][0] = std::to_string((int)log2(sizeArray[i]));

        for (int j = 0; j < (sizeof(assocArray) / sizeof(int)); j++)
        {
            Simulation sim(BLOCK_SIZE, sizeArray[i], assocArray[j], 0, 0, 1, 0, "traces/gcc_trace.txt");
            sim.run();

            double tat = (sim.getL1ReadCount() + sim.getL1WriteCount()) * accessTimeArray[i][j] + (sim.getL1ReadMissCount() + sim.getL1WriteMissCount()) * MISS_PENALTY;
            double aat = tat / (sim.getL1ReadCount() + sim.getL1WriteCount());

            // Graph #2
            // TAT = (L1_reads + L1_writes) * accessTime + (L1_read_misses + L1_write_misses) * miss_penalty
            dataArray[i + 1][j + 1] = std::to_string(aat);
        }
    }

    // need to format contents of dataArray into csv-ready string
    for (auto &row : dataArray)
    {
        for (auto &elem : row)
        {
            data.append(elem).append(",");
        }
        data.append("\n");
    }

    return data;
}

string getGraph3Data(int (&sizeArray)[11], double (&accessTimeArray)[5][11])
{
    int replacement_policy_array[3] = {0, 1, 2};
    string dataArray[12][4] = {{"Log2(SIZE)", "LRU", "FIFO", "Optimal"}};
    string data = "";

    for (int i = 0; i < (sizeof(sizeArray) / sizeof(int)); i++)
    {
        dataArray[i + 1][0] = std::to_string((int)log2(sizeArray[i]));

        for (int j = 0; j < (sizeof(replacement_policy_array) / sizeof(int)); j++)
        {
            Simulation sim(BLOCK_SIZE, sizeArray[i], 4, 0, 0, replacement_policy_array[j], 0, "traces/gcc_trace.txt");
            sim.run();

            double tat = (sim.getL1ReadCount() + sim.getL1WriteCount()) * accessTimeArray[i][j] + (sim.getL1ReadMissCount() + sim.getL1WriteMissCount()) * MISS_PENALTY;
            double aat = tat / (sim.getL1ReadCount() + sim.getL1WriteCount());

            // Total Access Time (TAT) Equation
            // TAT = (L1_reads + L1_writes) * accessTime + (L1_read_misses + L1_write_misses) * miss_penalty
            dataArray[i + 1][j + 1] = std::to_string(aat);
        }
    }

    // need to format contents of dataArray into csv-ready string
    for (auto &row : dataArray)
    {
        for (auto &elem : row)
        {
            data.append(elem).append(",");
        }
        data.append("\n");
    }

    return data;
}

string getGraph4Data(double (&accessTimeArray)[5][11])
{
    int inclusion_policy_array[2] = {0, 1};
    int localSizeArray[6] = {2048, 4096, 8192, 16384, 32768, 65536};
    string dataArray[7][3] = {{"Log2(SIZE)", "Inclusive", "Non-Inclusive"}};
    string data = "";

    for (int i = 0; i < (sizeof(localSizeArray) / sizeof(int)); i++)
    {
        dataArray[i + 1][0] = std::to_string((int)log2(localSizeArray[i]));

        for (int j = 0; j < (sizeof(inclusion_policy_array) / sizeof(int)); j++)
        {
            Simulation sim(BLOCK_SIZE, 1024, 4, localSizeArray[i], 8, 0, inclusion_policy_array[j], "traces/gcc_trace.txt");
            sim.run();

            double tat = (sim.getL1ReadCount() + sim.getL1WriteCount()) * accessTimeArray[i][j] + (sim.getL1ReadMissCount() + sim.getL1WriteMissCount()) * MISS_PENALTY;
            double aat = tat / (sim.getL1ReadCount() + sim.getL1WriteCount());

            // Total Access Time (TAT) Equation
            // TAT = (L1_reads + L1_writes) * accessTime + (L1_read_misses + L1_write_misses) * miss_penalty
            dataArray[i + 1][j + 1] = std::to_string(aat);
        }
    }

    // need to format contents of dataArray into csv-ready string
    for (auto &row : dataArray)
    {
        for (auto &elem : row)
        {
            data.append(elem).append(",");
        }
        data.append("\n");
    }

    return data;
}

int main(int argc, char *argv[])
{
    // if (argc != 9)
    // {
    //     std::cerr << "Usage: " << argv[0] << " <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_POLICY> <TRACE_FILE>\n";
    //     return 1;
    // }

    try
    {
        // unsigned int block_size = std::stoi(argv[1]);
        // unsigned int L1_size = std::stoi(argv[2]);
        // unsigned int L1_assoc = std::stoi(argv[3]);
        // unsigned int L2_size = std::stoi(argv[4]);
        // unsigned int L2_assoc = std::stoi(argv[5]);
        // unsigned int replacement_policy = std::stoi(argv[6]);
        // unsigned int inclusion_policy = std::stoi(argv[7]);
        // std::string trace_file = argv[8];

        // Simulation sim(block_size, L1_size, L1_assoc, L2_size, L2_assoc, replacement_policy, inclusion_policy, trace_file);
        // sim.run();

        int assocArray[5] = {1, 2, 4, 8, BLOCK_SIZE};
        int sizeArray[11] = {1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576};
        // Access time based on Association x Size
        double accessTimeArray[5][11] = {
            {0.114797, 0.12909, 0.147005, 0.16383, 0.198417, 0.233353, 0.294627, 0.3668, 0.443812, 0.563451, 0.69938},
            {0.140329, 0.161691, 0.181131, 0.194195, 0.223917, 0.262446, 0.300727, 0.374603, 0.445929, 0.567744, 0.706046},
            {0.14682, 0.154496, 0.185685, 0.211173, 0.233936, 0.27125, 0.319481, 0.38028, 0.457685, 0.564418, 0.699607},
            {0.180686, 0.189065, 0.212911, 0.254354, 0.288511, 0.341213, 0.401236, 0.458925, 0.578177, 0.705819},
            {0.155484, 0.176515, 0.182948, 0.198581, 0.205608, 0.22474, 0.276281, 0.322486, 0.396009, 0.475728, 0.588474}};
        string data = "";

        // data = getGraph1Data(assocArray, sizeArray);
        // data = getGraph2Data(assocArray, sizeArray, accessTimeArray);
        // data = getGraph3Data(sizeArray, accessTimeArray);
        data = getGraph4Data(accessTimeArray);

        cout << data << endl;
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
