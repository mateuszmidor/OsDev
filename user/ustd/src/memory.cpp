/**
 *   @file: memory.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "ScopeGuard.h"
#include "BumpAllocationPolicy.h"
#include "WyoosAllocationPolicy.h"

using namespace cstd::ustd;

namespace details {
    size_t heap_end {0};
    Mutex umalloc_mutex;

    /**
     * @brief   Request initial heap bytes and initialize memory allocation policy with it
     */
    cstd::BumpAllocationPolicy init_memory_management_policy() {
        constexpr size_t INIT_HEAP_BYTES {1024*1024};

        size_t heap_start = syscalls::brk(0);                   // brk(0) says where the current heap limit is
        heap_end = syscalls::brk(heap_start + INIT_HEAP_BYTES); // request more heap bytes
        return {heap_start, heap_end};
    }

    /**
     * @brief   Request additional heap bytes from the OS and return num bytes actually obtained
     */
    size_t request_additional_heap_bytes(size_t num_bytes) {
        size_t increase = num_bytes > 1024*1024 ? num_bytes : 1024*1024;    // at least 1 MB
        size_t heap_old_end = heap_end;
        heap_end = syscalls::brk(heap_old_end + increase);
        return heap_end - heap_old_end;
    }

    /**
     * @brief   policy does the actual management of heap memory pool obtained from the OS
     */
    auto policy = init_memory_management_policy();

    /**
     * @brief   Global user malloc
     */
    void* umalloc(size_t size) {
        ScopeGuard one_thread_at_a_time_here(umalloc_mutex);

        // try alloc from available heap
        if (void* ptr = policy.alloc_bytes(size))
            return ptr;

        // try request additional heap bytes from the system
        size_t increase = request_additional_heap_bytes(size);
        policy.extend_memory_pool(increase);

        // allocation should succeed unless the OS refused to give more heap bytes
        return policy.alloc_bytes(size);
    }

    /**
     * @brief   Global user free
     */
    void ufree(void* address) {
        ScopeGuard one_thread_at_a_time_here(umalloc_mutex);

        policy.free_bytes(address);
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
