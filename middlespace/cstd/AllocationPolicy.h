/**
 *   @file: AllocationPolicy.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_ALLOCATIONPOLICY_H_
#define SRC_MEMORY_ALLOCATIONPOLICY_H_

#include "types.h"

namespace cstd {

/**
 * @class   AllocationPolicy
 * @brief   This is a memory allocation policy interface, to be implemented by allocation policy of choice, eg. bump allocator
 */
class AllocationPolicy {
public:
    AllocationPolicy(size_t memory_start, size_t memory_end);   // continuous memory range to be used
    virtual ~AllocationPolicy() = default;

    // extend memory pool available  for allocation
    virtual void extend_memory_pool(size_t num_bytes) = 0;

    // alloc a block of memory and return its address;
    virtual void* alloc_bytes(size_t size) = 0;

    // free a block of memory
    virtual void free_bytes(void* address) = 0;

    // should be moved directly to MemoryManager?
    virtual size_t free_memory_in_bytes() = 0;
    virtual size_t total_memory_in_bytes() = 0;

protected:
    size_t available_memory_first_byte  = 0;    // first byte of physical memory available for allocation
    size_t available_memory_last_byte   = 0;    // last byte of physical memory available for allocation
};

} /* namespace cstd */

#endif /* SRC_MEMORY_ALLOCATIONPOLICY_H_ */
