/**
 *   @file: MemoryManager.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"

/**
 * @brief   Virtual memory address where the kernel is mapped
 * @see     boot.S
 */
extern size_t KERNEL_VIRTUAL_BASE;

namespace memory {

MemoryManager MemoryManager::_instance;
AllocationPolicy* MemoryManager::allocation_policy;

MemoryManager& MemoryManager::instance() {
    return _instance;
}

/**
 * @brief   Allocate and return a memory block virtual address, or return nullptr on failure
 */
void* MemoryManager::virt_alloc(size_t size) const {
    if (size_t physical_addr = (size_t)allocation_policy->alloc(size))
        return (void*)(physical_addr + KERNEL_VIRTUAL_BASE);
    else
        return nullptr;
}

/**
 * @brief   Release memory block located at virtual address
 */
void MemoryManager::virt_free(void* virtual_address) const {
    if (!virtual_address)
        return;

    size_t physical_addr = (size_t)virtual_address - KERNEL_VIRTUAL_BASE;
    allocation_policy->free((void*)physical_addr);
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
