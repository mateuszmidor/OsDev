/**
 *   @file: memory.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"

namespace details {
    /**
     * @brief   Global kernel malloc
     */
    void* kmalloc(size_t size) {
        memory::MemoryManager& mm =  memory::MemoryManager::instance();
        return mm.alloc_virt_memory(size);
    }

    /**
     * @brief   Global kernel free
     */
    void kfree(void* address) {
        memory::MemoryManager& mm =  memory::MemoryManager::instance();
        mm.free_virt_memory(address);
    }

} // details

void* operator new(size_t size) {
    return details::kmalloc(size);
}

void* operator new[](size_t size) {
    return details::kmalloc(size);
}

void operator delete[](void* ptr) {
    details::kfree(ptr);
}

void operator delete(void *ptr) {
    details::kfree(ptr);
}
