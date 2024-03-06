/*
Ethan Hymans
<Name>
<Name>
<Name>
<Name>

CDA5106 - Advanced Computer Architecture
Machine Problem 1: Cache Design, Memory Hierarchy Design
*/

/*
Arguments: <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY>
            <INCLUSION_PROPERTY> <trace_file>
*/
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

class CacheLine {
public:
    long long tag;
    bool dirty;
    CacheLine() : tag(-1), dirty(false) {}
};

class CacheSet {
public:
    vector<CacheLine> lines;
    vector<long long> lru_position;
    vector<long long> fifo_position;

    CacheSet(int assoc) : lines(assoc), lru_position(assoc), fifo_position(assoc+1, 0) {
        for(int i = 0; i < assoc; ++i) {
            lru_position[i] = i;
        }
    }
    // Methods for updating LRU, FIFO, and accessing/updating lines will be added here
};

class Cache {
private:
    vector<CacheSet> sets;
    unsigned long long num_sets;
    unsigned int assoc;
    unsigned int replacement_policy;
    unsigned int write_back_policy;

    unsigned long long hit_count = 0;
    unsigned long long miss_count = 0;
    unsigned long long reads_count = 0;
    unsigned long long writes_count = 0;

public:
    Cache(unsigned int size, unsigned int assoc, unsigned int replacement, unsigned int write_back) : 
        assoc(assoc), replacement_policy(replacement), write_back_policy(write_back) {
        num_sets = size / (64 * assoc);
        sets.resize(num_sets, CacheSet(assoc));
    }

    void simulate_access(char op, long long address);
    void update_lru(int set, int index);
    void update_fifo(int set, int index);
    // Additional methods for handling cache logic
    void print_statistics() {
        cout << static_cast<float>(miss_count) / (hit_count + miss_count) * 100 << "%\n";
        cout << writes_count << "\n";
        cout << reads_count << "\n";
    }
};

// Implementations of Cache::simulate_access, Cache::update_lru, Cache::update_fifo, and any additional required methods

class Simulation {
private:
    Cache cache;
    string trace_file;
public:
    Simulation(unsigned int block_size, unsigned int L1_size, unsigned int L1_assoc, unsigned int L2_size, unsigned int L2_assoc, unsigned int replacement_policy,
    unsigned int inclusion_policy, const string& trace_file) :
        cache(cache_size, assoc, replacement, write_back), trace_file(trace_file) {}

    void run() {
        ifstream inp(trace_file);
        if (!inp) {
            cerr << "Error opening trace file\n";
            return;
        }

        char op;
        long long address;
        while (inp >> op >> hex >> address) {
            cache.simulate_access(op, address);
        }

        inp.close();
        cache.print_statistics();
    }
};


/*////////////////////////////////////////////////
********************* MAIN **********************
///////////////////////////////////////////////*/
int main(int argc, char* argv[]) {
    if (argc != 8) {
        cerr << "Usage: <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_PROPERTY> <trace_file>\n";
        return 1;
    }

    unsigned int block_size = stoi(argv[0]);
    unsigned int L1_size = stoi(argv[1]);
    unsigned int L1_assoc = stoi(argv[2]);
    unsigned int L2_size = stoi(argv[3]);
    unsigned int L2_assoc = stoi(argv[4]);
    unsigned int replacement_policy = stoi(argv[5]);
    unsigned int inclusion_policy = stoi(argv[6]);
    string trace_file = argv[7];

    Simulation sim(block_size, L1_size, L1_assoc, L2_size, L2_assoc, replacement_policy, inclusion_policy, trace_file);
    sim.run();

    return 0;
}
