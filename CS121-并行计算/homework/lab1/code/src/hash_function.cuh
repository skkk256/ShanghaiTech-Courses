//
// Created by shengkuan on 2025/1/6.
//

#ifndef LAB1_HASH_FUNCTION_CUH
#define LAB1_HASH_FUNCTION_CUH

#include <cstdint>
#include "cuda.h"

#define PRIME1 2654435761U
#define PRIME2 2246822519U
#define PRIME3 3266489917U
#define PRIME4 668265263U
#define PRIME5 374761393U

__host__ __device__ inline uint32_t rotate_left(uint32_t v, uint32_t n)
{
    return (v << n) | (v >> (32 - n));
}

__host__ __device__ inline uint32_t xxhash(uint32_t seed, uint32_t v)
{
    uint32_t hash = seed + PRIME5;

    hash = hash + v * PRIME3;
    hash = rotate_left(hash, 17) * PRIME4;

    auto *byte = (uint8_t *)(&v);
    for (uint32_t i = 0; i < 4; i += 1)
    {
        hash = hash + byte[i] * PRIME5;
        hash = rotate_left(hash, 11) * PRIME1;
    }

    hash ^= hash >> 15;
    hash *= PRIME2;
    hash ^= hash >> 13;
    hash *= PRIME3;
    hash ^= hash >> 16;

    return hash;
}

#endif // LAB1_HASH_FUNCTION_CUH
