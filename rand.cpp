#include <cstdint>
#include <cstdlib>
#include <cmath> 
#include <iostream>

using namespace std;

typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;

// Based on pcg random number generator (https://pcg-random.org/)
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
void plat_get_entropy(void* data, u32 size) {
    BCryptGenRandom(NULL, static_cast<PUCHAR>(data), size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
}
#endif

class prng_state {
    u64 state;
    u64 inc;

    f32 prev_norm;
    bool has_prev_norm;

    public:
    prng_state(){
        u64 seeds[2] = {0};
        plat_get_entropy(seeds, sizeof(seeds));
        u64 initseq = seeds[0], initstate = seeds[1];
        this->state = 0U;
        this->inc = (initseq << 1u) | 1u;
        this->prng_rand_r();
        this->state += initstate;
        this->prng_rand_r();
        this->prev_norm = NAN;
        this->has_prev_norm = false;
    }

    u32 prng_rand_r(){
        u64 oldstate = this->state;
        this->state = oldstate * 6364136223846793005ULL + this->inc;
        u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        u32 rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    u32 operator()(){
        return this->prng_rand_r();
    }

    f32 prng_randf_r(){
        return (f32)this->prng_rand_r() / (f32)UINT32_MAX;
    }

    f32 prng_rand_norm_r(){ // uses box - muller transform
        if(this->has_prev_norm){
            f32 output = this->prev_norm;
            this->prev_norm = NAN;
            this->has_prev_norm = false;
            return output;
        }
        f32 u1 = 0.0f;
        do {
            u1 = this->prng_randf_r();
        } while (u1 == 0.0);
        f32 u2 = this->prng_randf_r();

        auto mag = sqrtf(-2.0 * logf(u1));
        auto z0  = mag * cosf(2.0 * M_PI * u2);
        auto z1  = mag * sinf(2.0 * M_PI * u2);
        this->prev_norm = z1;
        this->has_prev_norm = true;
        return z0;
    }

};

// int main(void) {
//     prng_state rng;
//     for (int i=0;i<10;i++){
//         cout<<rng()<<endl;
//     }
//     return 0;
// }

// g++ rand.cpp -o rand -lbcrypt ; if ($?) { .\rand }