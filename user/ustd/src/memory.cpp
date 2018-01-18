/**
 *   @file: memory.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "ScopeGuard.h"

using namespace cstd::ustd;

namespace details {
    size_t bump_addr = 0;
    size_t bump_old_limit = 0;
    size_t bump_limit = 0;
    Mutex umalloc_mutex;

    /**
     * @brief   Global user malloc
     */
    void* umalloc(size_t size) {
        ScopeGuard one_thread_at_a_time_here(umalloc_mutex);

        // setup dynamic memory allocation start
        if (bump_addr == 0) {
            bump_addr = syscalls::brk(0);
            bump_limit = bump_addr; // we start with 0 bytes of dynamic memory and then alloc as needed
        }

        // alloc dynamic memory to the program if needed
        if (bump_addr + size > bump_limit) {
            size_t increase = size > 1024*1024 ? size : 1024*1024;
            bump_old_limit = bump_limit;
            bump_limit = syscalls::brk(bump_old_limit + increase);
        }

        // if allocation failed - out of memory
        if (bump_limit == bump_old_limit)
            return nullptr;

        // cut out chunk of program dynamic memory
        size_t old_bump_addr = bump_addr;
        bump_addr+= size;
        return (void*)old_bump_addr;
    }

    /**
     * @brief   Global user free
     */
    void ufree(void* address) {
        // bump allocator doesnt free
    }
} // details


void* operator new(size_t size) {
    return details::umalloc(size);
}
void* operator new[](size_t size) {
    return details::umalloc(size);
}

void operator delete(void *ptr) noexcept {
    details::ufree(ptr);
}

void operator delete[](void* ptr) noexcept {
    details::ufree(ptr);
}
