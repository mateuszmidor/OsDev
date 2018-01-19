/**
 *   @file: BumpAllocationPolicy.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "BumpAllocationPolicy.h"

namespace cstd {

/**
 * Constructor.
 * @param   first_byte  The first byte of memory available for allocation
 * @param   last_byte   The last byte of memory available for allocation
 */
BumpAllocationPolicy::BumpAllocationPolicy(size_t first_byte, size_t last_byte):
        AllocationPolicy(first_byte, last_byte) {
    bump_addr = first_byte;
}

/**
 * @brief   Allocate a physical memory chunk that can hold "size" bytes, if enough memory is available.
 *          Corresponding physical memory frames will be allocated by OnPageFault handler.
 * @return  Allocated memory chunk physical address on success, nullptr on failure
 */
void* BumpAllocationPolicy::alloc_bytes(size_t size) {
    if (bump_addr + size > available_memory_last_byte)
        return nullptr;

    size_t old_bump_addr = bump_addr;
    bump_addr += size;
    return (void*)old_bump_addr;
}

void BumpAllocationPolicy::free_bytes(void* address) {
    // bump allocator doesnt free memory by design
}


size_t BumpAllocationPolicy::total_memory_in_bytes() {
    return available_memory_last_byte - available_memory_first_byte;
}

size_t BumpAllocationPolicy::free_memory_in_bytes() {
    return available_memory_last_byte - bump_addr;
}


} /* namespace cstd */
