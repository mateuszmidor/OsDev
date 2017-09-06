/**
 *   @file: BumpAllocator.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "BumpAllocationPolicy.h"

namespace memory {

BumpAllocator::BumpAllocator(size_t first_byte, size_t last_byte):
        AllocationPolicy(first_byte, last_byte) {
    bump_addr = first_byte;
}

/**
 * @brief   Allocate "size" bytes of memory if enough memory is available
 * @return  Allocated memory address on success, nullptr on failure
 */
void* BumpAllocator::alloc(size_t size) {
    if (bump_addr + size > available_memory_last_byte)
        return nullptr;

    size_t old = bump_addr;
    bump_addr += size;
    return (void*)old;
}

void BumpAllocator::free(void* address) {
    // bump allocator doesnt free memory by design
}

size_t BumpAllocator::total_memory_in_bytes() {
    return available_memory_last_byte - available_memory_first_byte;
}

size_t BumpAllocator::free_memory_in_bytes() {
    return available_memory_last_byte - bump_addr;
}


} /* namespace memory */
