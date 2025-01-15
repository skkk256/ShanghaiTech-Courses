#ifndef LAB1_BENCHMARK_CUH
#define LAB1_BENCHMARK_CUH

#include <vector>
#include <chrono>
#include <iostream>
#include <cmath>
#include "cuckoo_hash_table.cuh"
#include "utils.cuh"

#define DISPLAY_CAPACITY false

const uint32_t TABLESIZE = 1 << 25;

class Benchmark
{
public:
    static void experiment1(int nStart, int nEnd, int repeatTimes, uint32_t hashFunctions)
    {
        std::cout << "Experiment 1: Insert 2^n keys" 
                << ", [" << nStart  << ", " << nEnd << "]" 
                << ", use " << hashFunctions << " hash functions"
                << std::endl; 
        for (int scale = nStart; scale <= nEnd; ++scale)
        {
            int numKeys = 1 << scale;
            // uint32_t *keys = new uint32_t[numKeys];
            // generate_unique_random(keys, numKeys);
            std::vector<uint32_t> keys = generate_unique_random(numKeys);

            double totalDuration = 0;
            for (int i = 0; i < repeatTimes; ++i)
            {
                // CuckooHashTable hashTable(TABLESIZE, clog2(numKeys), hashFunctions);
                bool use_three_hash = hashFunctions == 3;
                CuckooHashTable hashTable(TABLESIZE, clog2(numKeys), use_three_hash);
                double duration = time_func([&]
                                            { hashTable.insert(keys); });
                totalDuration += duration;
                double throughput = numKeys / duration / 1e6;

                if (DISPLAY_CAPACITY)
                {
                    hashTable.display(false);
                }
            }

            double avgDuration = totalDuration / repeatTimes;
            double avgThroughput = numKeys / avgDuration / 1e6;
            std::cout << "[ n = " << scale << " ], Average Time = " << avgDuration << "s, Average Throughput = "
                    << avgThroughput << " M ops/s" << std::endl;        
            // delete[] keys;
        }
    }

    static void experiment2(int iStart, int iEnd, int numLookups, int repeatTimes, uint32_t hashFunctions)
    {
        std::cout << "Experiment 2: Lookup Performance with Varying Key Sets"
                  << ", numLookups = " << numLookups
                  << ", [" << 100 - 10 * iStart << "%, " << 100 - 10 * iEnd << "%]"
                  << ", use " << hashFunctions << " hash functions"
                  << std::endl;

        uint32_t numKeys = 1 << 24;

        for (int percent = iStart; percent <= iEnd; ++percent)
        {
            std::vector<int> results(numKeys);
            std::vector<uint32_t> insert_values(numKeys);
            std::vector<uint32_t> lookup_values(numKeys);

            int bound = ceil((1 - 0.1 * percent) * numKeys);

            double totalDuration = 0;
            for (int r = 0; r < repeatTimes; ++r)
            {
                for (int i = 0; i < numKeys; ++i)
                {
                    insert_values[i] = i + 1;
                }

                for (int i = 0; i < bound; ++i)
                {
                    lookup_values[i] = insert_values[i];
                }
                for (int i = bound; i < numKeys; ++i)
                {
                    lookup_values[i] = numKeys + i;
                }
                bool use_three_hash = hashFunctions == 3;
                CuckooHashTable hashTable(TABLESIZE, clog2(numKeys), use_three_hash);
                hashTable.insert(insert_values);

                double duration = time_func([&]
                                            { results = hashTable.lookup(lookup_values); });
                totalDuration += duration;

                double throughput = numKeys / duration / 1e6;
            }

            double avgDuration = totalDuration / repeatTimes;
            double avgThroughput = numKeys / avgDuration / 1e6;
            std::cout << "[ p = " << 100 - 10 * percent << " ], Average Time = " << avgDuration << "s, Average Throughput = "
                      << avgThroughput << " M ops/s" << std::endl;

        }
    }

    static void experiment3(int repeatTimes, uint32_t hashFunctions)
    {
        std::cout << "Experiment 3: Insertion Time vs. Table Size"
                  << ", use " << hashFunctions << " hash functions"
                  << std::endl;

        uint32_t numKeys = 1 << 24;
        std::vector<uint32_t> keys = generate_unique_random(numKeys);


        float sps[] = {2.0, 1.9, 1.8, 1.7, 1.6, 1.5, 1.4, 1.3, 1.2, 1.1, 1.05, 1.02, 1.01};

        for (auto sp : sps)
        {

            double totalDuration = 0;
            uint32_t newTableSize = ceil(sp * numKeys);
            for (int i = 0; i < repeatTimes; ++i)
            {
                bool use_three_hash = hashFunctions == 3;

                CuckooHashTable hashTable(newTableSize, clog2(numKeys), use_three_hash);

                double duration = time_func([&]
                                            { hashTable.insert(keys); });
                totalDuration += duration;

                double throughput = numKeys / duration / 1e6;

                if (DISPLAY_CAPACITY)
                {
                    hashTable.display(false);
                }
            }

            double avgDuration = totalDuration / repeatTimes;
            double avgThroughput = numKeys / avgDuration / 1e6;
            std::cout << "[ size = " << sp << "n ], Average Time = " << avgDuration << "s, Average Throughput = "
                    << avgThroughput << " M ops/s" << std::endl;   

        }
        // delete[] insert_values;
    }

    static void experiment4(int repeatTimes, uint32_t hashFunctions)
    {
        std::cout << "Experiment 4: Eviction Bound Impact on Insertion Time"
                  << ", use " << hashFunctions << " hash functions"
                  << std::endl;

        uint32_t numKeys = 1 << 24;
        // uint32_t *insert_values = new uint32_t[numKeys];
        // generate_unique_random(insert_values, numKeys);
        std::vector<uint32_t> keys = generate_unique_random(numKeys);


        uint32_t newTableSize = ceil(1.4 * numKeys);

        for (int evict_bound = 1; evict_bound <= 10; ++evict_bound)
        {

            double totalDuration = 0;
            for (int i = 0; i < repeatTimes; ++i)
            {
                bool use_three_hash = hashFunctions == 3;

                CuckooHashTable hashTable(newTableSize, evict_bound * clog2(numKeys), use_three_hash);

                double duration = time_func([&]
                                            { hashTable.insert(keys); });

                totalDuration += duration;
                double throughput = numKeys / duration / 1e6;
                if (DISPLAY_CAPACITY)
                {
                    hashTable.display(false);
                }

            }

            double avgDuration = totalDuration / repeatTimes;
            double avgThroughput = numKeys / avgDuration / 1e6;
            std::cout << "[ evict bound = " << evict_bound << "x ], Average Time = " << avgDuration << "s, Average Throughput = "
                    << avgThroughput << " M ops/s" << std::endl;   

        }

        // delete[] insert_values;
    }

};

#endif // LAB1_BENCHMARK_CUH
