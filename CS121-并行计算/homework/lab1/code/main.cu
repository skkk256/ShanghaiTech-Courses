#include <iostream>
#include "benchmark.cuh" // Ensure this path is correct

int main()
{
    // Define the number of repetitions for each experiment
    int repeatTimes = 5;

    // Experiment 1: Insertion of 2^n keys
    // First with two hash functions
    Benchmark::experiment1(8, 24, repeatTimes, 2);
    // Then with three hash functions
    Benchmark::experiment1(8, 24, repeatTimes, 3);

    std::cout << "--------------------------------------------------" << std::endl;

    // Experiment 2: Lookup Performance with Varying Key Sets
    // First with two hash functions
    Benchmark::experiment2(0, 10, 1 << 24, repeatTimes, 2);
    // Then with three hash functions
    Benchmark::experiment2(0, 10, 1 << 24, repeatTimes, 3);

    std::cout << "--------------------------------------------------" << std::endl;

    // Experiment 3: Insertion Time vs. Table Size
    // First with two hash functions
    Benchmark::experiment3(repeatTimes, 2);
    // Then with three hash functions
    Benchmark::experiment3(repeatTimes, 3);

    // std::cout << "--------------------------------------------------" << std::endl;

    // Experiment 4: Eviction Bound Impact on Insertion Time
    // First with two hash functions
    Benchmark::experiment4(repeatTimes, 2);
    // Then with three hash functions
    Benchmark::experiment4(repeatTimes, 3);

    return 0;
}
