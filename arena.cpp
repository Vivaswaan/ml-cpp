#include <cstdint>
#include <cstdlib>
#include <cstring> 
#include <algorithm> 
#include "arena.h"

#ifdef _WIN32
#include<windows.h>
#include <cstdio>

#define Kib(n) ((u64)(n)<<10)
#define Mib(n) ((u64)(n)<<20)
#define Gib(n) ((u64)(n)<<30)

u32 plat_get_pagesize(void) {
    SYSTEM_INFO sysinfo = {0};
    GetSystemInfo(&sysinfo);

    return sysinfo.dwPageSize;
}
void* plat_mem_reserve(u64 size) {
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}
b32 plat_mem_commit(void* ptr, u64 size) {
    void* ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return ret != nullptr;
}
b32 plat_mem_decommit(void* ptr, u64 size) {
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}
b32 plat_mem_release(void* ptr, u64 size) {
    return VirtualFree(ptr, size, MEM_RELEASE);
}
#endif

mem_arena* arena_create(u64 reserve_size, u64 commit_size){
    u32 pagesize = plat_get_pagesize();
    reserve_size = ALIGN_UP_POW2(reserve_size, pagesize);
    commit_size = ALIGN_UP_POW2(commit_size, pagesize);    
    mem_arena* arena = static_cast<mem_arena*>(plat_mem_reserve(reserve_size));

    if(!plat_mem_commit(arena, commit_size)){
        return nullptr;
    }

    arena->reserve_size = reserve_size;
    arena->commit_size = commit_size;
    arena->position = ARENA_BASE_POS;
    arena->commit_position = commit_size;

    return arena;
}
void arena_destroy(mem_arena* arena){
    plat_mem_release(arena, arena->reserve_size);
}
void* arena_push(mem_arena* arena, u64 size, b32 nonzero){
    u64 position_aligned = ALIGN_UP_POW2(arena->position, ARENA_ALIGN);
    u64 new_position = position_aligned + size;
    if(new_position > arena->reserve_size) {
        return nullptr;
    }
    if(new_position > arena->commit_position) {
        u64 new_commit_position = new_position;
        new_commit_position += (arena->commit_size - 1);
        new_commit_position -= (new_commit_position % arena->commit_size);
        new_commit_position = MIN(new_commit_position, arena->reserve_size);

        u8* memory = (u8*)arena + arena->commit_position;
        u64 commit_size = new_commit_position - arena->commit_position;

        if(!plat_mem_commit(memory, commit_size)) {
            return nullptr;
        }

        arena->commit_position = new_commit_position;
    }
    
    arena->position = new_position;
    u8* out = (u8*)arena + position_aligned;

    if(!nonzero) {
        memset(out, 0, size);
    }

    return out;
}
void arena_pop(mem_arena* arena, u64 size){
    size = MIN(size, arena->position-ARENA_BASE_POS);
    arena->position-=size; 
}
void arena_pop_to(mem_arena* arena, u64 position){
    u64 size = position < arena->position ? arena->position - position : 0;
    arena_pop(arena,size);
}
void arena_clear(mem_arena* arena){
    arena_pop_to(arena, ARENA_BASE_POS);
}

static __thread mem_arena* _scratch_arenas[2] = { NULL, NULL };

mem_arena_temp arena_scratch_get(mem_arena** conflicts, u32 num_conflicts) {
    i32 scratch_index = -1;

    for (i32 i = 0; i < 2; i++) {
        b32 conflict_found = false;

        for (u32 j = 0; j < num_conflicts; j++) {
            if (_scratch_arenas[i] == conflicts[j]) {
                conflict_found = true;
                break;
            }
        }

        if (!conflict_found) {
            scratch_index = i;
            break;
        }
    }

    if (scratch_index == -1) {
        return (mem_arena_temp){ 0 };
    }

    mem_arena** selected = &_scratch_arenas[scratch_index];

    if (*selected == NULL) {
        *selected = arena_create(MiB(64), MiB(1));
    }

    return arena_temp_begin(*selected);
}

void arena_scratch_release(mem_arena_temp scratch) {
    arena_temp_end(scratch);
}

// int main() {
//     mem_arena* perm_arena = arena_create(Mib(1), Kib(1));
//     while(1) {
//         arena_push(perm_arena, Kib(16), false);
//         getc(stdin);
//     }
//     arena_destroy(perm_arena); 
//     return 0;
// }
 