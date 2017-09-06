/**
 *   @file: MemoryManager.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"

void* kmalloc(size_t size) {
    memory::MemoryManager& mm = memory::MemoryManager::instance();
    return mm.alloc(size);
}

void kfree(void* address) {
    memory::MemoryManager& mm = memory::MemoryManager::instance();
    mm.free(address);
}


namespace memory {

MemoryManager MemoryManager::_instance;
AllocationPolicy* MemoryManager::allocation_policy;

MemoryManager& MemoryManager::instance() {
    return _instance;
}

void* MemoryManager::alloc(size_t size) const {
    return allocation_policy->alloc(size);
}

void MemoryManager::free(void* address) const {
    allocation_policy->free(address);
}

/**
 * @brief   Set the last byte available for allocation.
 *          Use is when the available memory range is known (eg. extracted from multiboot2 structures)
 */
void MemoryManager::set_high_limit(size_t limit) const {
    allocation_policy->set_high_limit_in_bytes(limit);
}

/**
 * @brief   Get total memory amount available for dynamic allocation.
 *          This is less than total physical memory since the first megabyte is used for multiboot2 structures
 *          and the second megabyte is used for kernel code and data; see MemoryManager::install_allocation_policy
 */
size_t MemoryManager::get_total_memory_in_bytes() const {
    return allocation_policy->total_memory_in_bytes();
}

/**
 * @brief   Get amount of memory left for dynamic allocation.
 */
size_t MemoryManager::get_free_memory_in_bytes() const {
    return allocation_policy->free_memory_in_bytes();
}

} /* namespace memory */
