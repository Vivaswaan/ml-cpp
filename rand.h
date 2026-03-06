#pragma once
#include <cstdint>

typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;

class prng_state {
    u64 state;
    u64 inc;

    f32  prev_norm;
    bool has_prev_norm;

public:
    prng_state();

    u32 prng_rand_r();
    u32 operator()();

    f32 prng_randf_r();
    f32 prng_rand_norm_r();
};