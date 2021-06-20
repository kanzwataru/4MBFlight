#pragma once
#include "common.h"

static uint32_t g_rand_seed = 0xF00D1337;

uint32_t rand_xorshift_32(uint32_t &seed = g_rand_seed)
{
    uint32_t x = g_rand_seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    seed = x;
    return x;
}

static inline float rand_range(float min, float max)
{
    const uint32_t i_val = rand_xorshift_32();
    const float normalized = float(i_val) / float(0xFFFFFFFF);

    return normalized * (max - min) + min;
}

// Can use this to bypass actual rand stuff above
static inline float remap_to_float(float min, float max, uint16_t i_val)
{
    //assert(min <= max);

    const float normalized = float(i_val) / float(0xFFFF);
    return normalized * (max - min) + min;
}
