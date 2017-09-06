/**
 *   @file: MemoryManager.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"

namespace memory {

MemoryManager MemoryManager::_instance;
AllocationPolicy* MemoryManager::allocation_policy;

MemoryManager& MemoryManager::instance() {
    return _instance;
}

/**
 * @brief   Allocate and return a memory block, or return nullptr on failure
 */
void* MemoryManager::alloc(size_t size) const {
    return allocation_policy->alloc(size);
}

/**
 * @brief   Release memory block at "address"
 */
void MemoryManager::free(void* address) const {
    allocation_policy->free(address);
}

/**
 * @brief   Get amount of memory left for dynamic allocation.
 */
size_t MemoryManager::get_free_memory_in_bytes() const {
    return allocation_policy->free_memory_in_bytes();
}

/**
 * @brief   Get total memory amount available for dynamic allocation.
 *          This is less than total physical memory,
 *          since the first megabytes are used for kernel code/data and multiboot2 structures (eg. 0..2MB in release build, 0..7MB in debug build)
 */
size_t MemoryManager::get_total_memory_in_bytes() const {
    return allocation_policy->total_memory_in_bytes();
}


} /* namespace memory */
