#pragma once
#include <cstdint>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 b8;
typedef i32 b32;

#define Kib(n) ((u64)(n)<<10)
#define Mib(n) ((u64)(n)<<20)
#define Gib(n) ((u64)(n)<<30)

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#define ALIGN_UP_POW2(n,p) (((u64)(n) + ((u64)(p)-1)) & (~((u64)(p)-1)))
#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN (sizeof(void*))
#define PUSH_STRUCT(arena, T) (T*)arena_push(arena, sizeof(T), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push(arena, sizeof(T), true)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push(arena, sizeof(T)*(n), false)
#define PUSH_ARRAY_NZ(arena, T, n) (T*)arena_push(arena, sizeof(T)*(n), true)

typedef struct {
    u64 reserve_size;
    u64 commit_size;
    u64 position;
    u64 commit_position;
} mem_arena;

u32  plat_get_pagesize(void);
void* plat_mem_reserve(u64 size);
b32  plat_mem_commit(void* ptr, u64 size);
b32  plat_mem_decommit(void* ptr, u64 size);
b32  plat_mem_release(void* ptr, u64 size);
mem_arena* arena_create(u64 reserve_size, u64 commit_size);
void arena_destroy(mem_arena* arena);
void* arena_push(mem_arena* arena, u64 size, b32 nonzero);
void  arena_pop(mem_arena* arena, u64 size);
void  arena_pop_to(mem_arena* arena, u64 position);
void  arena_clear(mem_arena* arena);

class mem_arena_temp {
    public:
    mem_arena* arena;
    u64 start_pos;
};

mem_arena_temp arena_temp_begin(mem_arena* arena);
void arena_temp_end(mem_arena_temp temp);

mem_arena_temp arena_scratch_get(mem_arena** conflicts, u32 num_conflicts);
void arena_scratch_release(mem_arena_temp scratch);