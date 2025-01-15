//
// Created by shengkuan on 2025/1/6.
//

#ifndef LAB1_UTILS_CUH
#define LAB1_UTILS_CUH

#include <functional>
#include <chrono>
#include <ctime>
#include <set>
#include <random>
#define clog2(x) ceil(log2((double)x))

template <typename T>
void do_swap(T &a, T &b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

inline double time_func(std::function<void()> f)
{

    auto start = std::chrono::system_clock::now();
    f();
    auto end = std::chrono::system_clock::now();
    auto nano = std::chrono::duration<double>(end - start).count();

    return (double)(nano);
}

std::vector<uint32_t> generate_unique_random(uint32_t num)
{
    std::set<uint32_t> unique_keys;
    std::vector<uint32_t> keys;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, num * 10);

    // Generate unique random keys
    while (keys.size() < num)
    {
        uint32_t key = dis(gen);
        if (unique_keys.insert(key).second)
        {
            keys.push_back(key);
        }
    }

    return keys;
}

// std::vector<uint32_t> generate_random_keys(int numKeys, int seed = 0)
// {
//     std::vector<uint32_t> keys(numKeys);
//     std::mt19937 rng(seed);
//     std::uniform_int_distribution<int> dist(0, INT32_MAX);
//     for (uint32_t &key : keys)
//     {
//         key = dist(rng);
//     }
//     return keys;
// }


#endif
