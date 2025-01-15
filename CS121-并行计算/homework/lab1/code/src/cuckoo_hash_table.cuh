#ifndef CUCKOO_HASH_TABLE_CUH
#define CUCKOO_HASH_TABLE_CUH

#include <vector>
#include <iostream>
#include <random>
#include "cuda_runtime.h"
#include "hash_function.cuh"

#define EMPTY_CELL (0)
#define SEED1 12123214U
#define SEED2 1006224070U
#define SEED3 42U
// #define USE_THREE_HASH false
#define BLOCK_SIZE 256

__device__ __host__ inline int compute_hash(uint32_t seed, uint32_t key, int size)
{
    return xxhash(seed, key) % size;
}

__device__ __host__ inline int pseudo_random(int seed, int step) {
    return (seed * 1664525 + 1013904223 + step) & 0x7FFFFFFF;
}


__device__ __host__ uint32_t hash0(uint32_t key, uint32_t size) {
    return key % size;
}


__global__ void insertKernel(uint32_t *d_keys, uint32_t *d_table, uint32_t size, uint32_t num_keys, uint32_t evict_bound, bool use_three_hash) {
    uint32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_keys) return;

    uint32_t key = d_keys[idx];
    uint32_t pos1 = hash0(key, size);
    uint32_t pos2 = compute_hash(SEED2, key, size);
    uint32_t pos3 = use_three_hash ? compute_hash(SEED3, key, size) : EMPTY_CELL;

    if (atomicCAS(&d_table[pos1], EMPTY_CELL, key) == EMPTY_CELL) {
        return;
    }

    if (atomicCAS(&d_table[pos2], EMPTY_CELL, key) == EMPTY_CELL) {
        return;
    }

    if (use_three_hash && pos3 != EMPTY_CELL) {
        if (atomicCAS(&d_table[pos3], EMPTY_CELL, key) == EMPTY_CELL) {
            return;
        }
    }

    uint32_t current_key = key;
    uint32_t current_pos = pos1;
    for (uint32_t i = 0; i < evict_bound; ++i) {
        uint32_t displaced = atomicExch(&d_table[current_pos], current_key);
        if (displaced == current_key) {
            return;
        }

        current_key = displaced;

        uint32_t new_pos1 = hash0(current_key, size);
        uint32_t new_pos2 = compute_hash(SEED2, current_key, size);
        uint32_t new_pos3 = use_three_hash ? compute_hash(SEED3, current_key, size) : EMPTY_CELL;

        if (atomicCAS(&d_table[new_pos1], EMPTY_CELL, current_key) == EMPTY_CELL) {
            return;
        }

        if (atomicCAS(&d_table[new_pos2], EMPTY_CELL, current_key) == EMPTY_CELL) {
            return;
        }

        if (use_three_hash && new_pos3 != EMPTY_CELL) {
            if (atomicCAS(&d_table[new_pos3], EMPTY_CELL, current_key) == EMPTY_CELL) {
                return;
            }
        }

        current_pos = new_pos1;
    }

}


__global__ static void lookupKernel(uint32_t *keys, int *results, uint32_t *table, int size, int numKeys, bool use_three_hash)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numKeys)
        return;

    uint32_t key = keys[idx];
    int pos1 = hash0(key, size);
    int pos2 = compute_hash(SEED2, key, size);
    int pos3 = use_three_hash ? compute_hash(SEED3, key, size) : -1;

    if (table[pos1] == key)
    {
        results[idx] = 1;
        return;
    }

    if (table[pos2] == key)
    {
        results[idx] = 1;
        return;
    }

    if (use_three_hash && table[pos3] == key)
    {
        results[idx] = 1;
        return;
    }

    results[idx] = 0;
}

__global__ void removeKernel(uint32_t *d_keys, uint32_t *d_table, uint32_t size, uint32_t num_keys, bool use_three_hash) {
    uint32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_keys) return;

    uint32_t key = d_keys[idx];
    uint32_t pos1 = hash0(key, size);
    uint32_t pos2 = compute_hash(SEED2, key, size);
    uint32_t pos3 = use_three_hash ? compute_hash(SEED3, key, size) : EMPTY_CELL;

    if (atomicCAS(&d_table[pos1], key, EMPTY_CELL) == key) {
        return;
    }

    if (atomicCAS(&d_table[pos2], key, EMPTY_CELL) == key) {
        return;
    }

    if (use_three_hash && pos3 != EMPTY_CELL) {
        atomicCAS(&d_table[pos3], key, EMPTY_CELL);
    }
}

class CuckooHashTable
{
private:
    uint32_t *d_table;
    uint32_t size;
    uint32_t evict_bound;
    bool use_three_hash;

public:
    CuckooHashTable(uint32_t size, uint32_t evict_bound, bool use_three_hash) : 
        size(size), evict_bound(evict_bound), use_three_hash(use_three_hash)
    {
        cudaMalloc(&d_table, size * sizeof(uint32_t));
        cudaMemset(d_table, EMPTY_CELL, size * sizeof(uint32_t));
    }

    ~CuckooHashTable()
    {
        cudaFree(d_table);
    }

    void insert(const std::vector<uint32_t> &keys)
    {
        uint32_t *d_keys;
        size_t bytes = keys.size() * sizeof(uint32_t);

        cudaMalloc(&d_keys, bytes);

        cudaMemcpy(d_keys, keys.data(), bytes, cudaMemcpyHostToDevice);

        int numBlocks = (keys.size() + BLOCK_SIZE - 1) / BLOCK_SIZE;

        insertKernel<<<numBlocks, BLOCK_SIZE>>>(d_keys, d_table, size, keys.size(), evict_bound, use_three_hash);
        cudaDeviceSynchronize();

        cudaFree(d_keys);
    }

    // New Remove Method
    void remove(const std::vector<uint32_t> &keys)
    {
        uint32_t *d_keys;
        size_t bytes = keys.size() * sizeof(uint32_t);

        cudaMalloc(&d_keys, bytes);

        cudaMemcpy(d_keys, keys.data(), bytes, cudaMemcpyHostToDevice);

        int numBlocks = (keys.size() + BLOCK_SIZE - 1) / BLOCK_SIZE;

        removeKernel<<<numBlocks, BLOCK_SIZE>>>(d_keys, d_table, size, keys.size(), use_three_hash);
        cudaDeviceSynchronize();

        cudaFree(d_keys);
    }

    std::vector<int> lookup(const std::vector<uint32_t> &keys)
    {
        uint32_t *d_keys;
        int *d_results;
        std::vector<int> results(keys.size());
        size_t bytes = keys.size() * sizeof(uint32_t);

        cudaMalloc(&d_keys, bytes);
        cudaMalloc(&d_results, keys.size() * sizeof(int)); // Corrected bytes for results

        cudaMemcpy(d_keys, keys.data(), bytes, cudaMemcpyHostToDevice);

        int numBlocks = (keys.size() + BLOCK_SIZE - 1) / BLOCK_SIZE;

        lookupKernel<<<numBlocks, BLOCK_SIZE>>>(d_keys, d_results, d_table, size, keys.size(), use_three_hash);
        cudaDeviceSynchronize();

        cudaMemcpy(results.data(), d_results, keys.size() * sizeof(int), cudaMemcpyDeviceToHost); // Corrected bytes for results

        cudaFree(d_keys);
        cudaFree(d_results);

        return results;
    }

    void display(bool print_table = false)
    {
        std::vector<uint32_t> h_table(size);

        cudaMemcpy(h_table.data(), d_table, size * sizeof(uint32_t), cudaMemcpyDeviceToHost);

        int occupied = 0;
        for (auto v : h_table)
        {
            if (v != EMPTY_CELL)
                occupied++;
        }

        float load_factor = (static_cast<float>(occupied) / size) * 100.0f;

        std::cout << "CuckooHashTable Info:\n";
        std::cout << "Size: " << size << "\n";
        std::cout << "Occupied Cells: " << occupied << "/" << size << " (" << load_factor << "%)\n";

        if (print_table)
        {
            std::cout << "Table: ";
            for (auto v : h_table)
                std::cout << v << " ";
            std::cout << std::endl;
        }
    }
};

#endif
