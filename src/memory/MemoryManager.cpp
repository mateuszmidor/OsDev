/**
 *   @file: MemoryManager.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"
#include "HigherHalf.h"

namespace memory {

MemoryManager MemoryManager::_instance;
AllocationPolicy* MemoryManager::allocation_policy;

/**
 * @brief   Global kernel malloc
 */
void* kmalloc(size_t size) {
    MemoryManager& mm = MemoryManager::instance();
    return mm.alloc_virt_memory(size);
}

/**
 * @brief   Global kernel free
 */
void kfree(void* address) {
    MemoryManager& mm = MemoryManager::instance();
    mm.free_virt_memory(address);
}

MemoryManager& MemoryManager::instance() {
    return _instance;
}

/**
 * @brief   Allocate a contiguous, frame-size aligned block of physical memory that can hold "size" bytes, or return nullptr on failure
 * @note    Allocated memory chunk physical address on success, nullptr on failure
 */
void* MemoryManager::alloc_frames(size_t size) const {
    ssize_t phys_addr = FrameAllocator::alloc_consecutive_frames(size);
    if (phys_addr == -1)
        return nullptr;
    else
        return (void*)phys_addr;
}

/**
 * @brief   Release memory block of size "size" located at physical address
 *          A number of frames necessary to hold "size" bytes will be freed
 */
void MemoryManager::free_frames(void* address, size_t size) const {
    FrameAllocator::free_consecutive_frames((size_t)address, size);
}

/**
 * @brief   Allocate and return a memory block virtual address, or return nullptr on failure
 * @note    Memory frames will be allocated by PageFaulHandler
 */
void* MemoryManager::alloc_virt_memory(size_t size) const {
    return allocation_policy->alloc_bytes(size);
}

/**
 * @brief   Release memory block located at virtual address
 */
void MemoryManager::free_virt_memory(void* virtual_address) const {
    if (!virtual_address)
        return;

    allocation_policy->free_bytes((void*)virtual_address);
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
