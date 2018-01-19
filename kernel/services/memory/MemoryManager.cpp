/**
 *   @file: MemoryManager.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "MemoryManager.h"
#include "HigherHalf.h"
#include "KLockGuard.h"

using namespace multitasking;

namespace memory {

MemoryManager MemoryManager::_instance;
cstd::AllocationPolicy* MemoryManager::allocation_policy;

MemoryManager& MemoryManager::instance() {
    return _instance;
}

/**
 * @brief   Allocate a contiguous, frame-size aligned block of physical memory that can hold "size" bytes, or return nullptr on failure
 * @note    Allocated memory chunk physical address on success, nullptr on failure
 */
void* MemoryManager::alloc_frames(size_t size) const {
    KLockGuard lock;    // prevent reschedule

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
    KLockGuard lock;    // prevent reschedule

    FrameAllocator::free_consecutive_frames((size_t)address, size);
}

/**
 * @brief   Allocate and return a memory block virtual address, or return nullptr on failure
 * @note    Memory frames will be allocated by PageFaulHandler
 */
void* MemoryManager::alloc_virt_memory(size_t size) const {
    KLockGuard lock;    // prevent reschedule

    return allocation_policy->alloc_bytes(size);
}

/**
 * @brief   Release memory block located at virtual address
 */
void MemoryManager::free_virt_memory(void* virtual_address) const {
    KLockGuard lock;    // prevent reschedule

    if (!virtual_address)
        return;

    allocation_policy->free_bytes((void*)virtual_address);
}

/**
 * @brief   Get amount of memory left for dynamic allocation.
 */
size_t MemoryManager::get_free_memory_in_bytes() const {
    KLockGuard lock;    // prevent reschedule

    return allocation_policy->free_memory_in_bytes();
}

/**
 * @brief   Get total memory amount available for dynamic allocation.
 *          This is less than total physical memory,
 *          since the first megabytes are used for kernel code/data and multiboot2 structures (eg. 0..2MB in release build, 0..7MB in debug build)
 */
size_t MemoryManager::get_total_memory_in_bytes() const {
    KLockGuard lock;    // prevent reschedule

    return allocation_policy->total_memory_in_bytes();
}


} /* namespace memory */
