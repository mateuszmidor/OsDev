/**
 *   @file: BumpAllocationPolicy.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "FrameAllocator.h"
#include "BumpAllocationPolicy.h"

namespace memory {

/**
 * Constructor.
 * @param   first_byte  The first byte of physical memory available for allocation
 * @param   last_byte   The last byte of physical memory available for allocation
 */
BumpAllocationPolicy::BumpAllocationPolicy(size_t first_byte, size_t last_byte):
        AllocationPolicy(first_byte, last_byte) {
    bump_phys_addr = first_byte;
}

/**
 * @brief   Allocate a physical memory chunk that can hold "size" bytes, if enough memory is available.
 *          Corresponding physical memory frames will be allocated by OnPageFault handler.
 * @return  Allocated memory chunk physical address on success, nullptr on failure
 */
void* BumpAllocationPolicy::alloc_bytes(size_t size) {
    if (bump_phys_addr + size > available_memory_last_byte)
        return nullptr;

    size_t old_bump_addr = bump_phys_addr;
    bump_phys_addr += size;
    return (void*)old_bump_addr;
}

void BumpAllocationPolicy::free_bytes(void* address) {
    // bump allocator doesnt free memory by design
}

/**
 * @brief   Allocate a chain of contiguous memory frames that can hold "size" bytes, if enough frames is available.
 *          The physical memory frames are allocated immediately.
 *          This method is good for allocating low-level CPU structs eg PageTables64
 * @return  Allocated memory frames physical address on success, nullptr on failure
 */
void* BumpAllocationPolicy::alloc_frames(size_t size) {
//    const size_t size_rounded_to_frames = (size / FrameAllocator::get_frame_size() + 1) * FrameAllocator::get_frame_size();
//    // try alloc a series of physical frames
//    ssize_t phys_addr = FrameAllocator::alloc_consecutive_frames(size);
//    if (phys_addr == -1)
//        return nullptr;
//
//    // reflect the fact the frames have been allocated in bump_phys_addr, so no alloc_bytes will return memory that correspond to our frames
//    if (bump_phys_addr < phys_addr + size_rounded_to_frames)
//        bump_phys_addr = phys_addr + size_rounded_to_frames;
//
//    return (void*)phys_addr;
    return nullptr;
}

void BumpAllocationPolicy::free_frames(void* address, size_t size) {
    // release physical frames
    FrameAllocator::free_consecutive_frames((size_t)address, size);

    // reflect the fact that frames have been released in memory allocation scheme for alloc_bytes; not suitable for bump allocator that never frees
}

size_t BumpAllocationPolicy::total_memory_in_bytes() {
    return available_memory_last_byte - available_memory_first_byte;
}

size_t BumpAllocationPolicy::free_memory_in_bytes() {
    return available_memory_last_byte - bump_phys_addr;
}


} /* namespace memory */
