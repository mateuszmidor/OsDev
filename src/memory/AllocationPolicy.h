/**
 *   @file: Allocator.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_ALLOCATIONPOLICY_H_
#define SRC_MEMORY_ALLOCATIONPOLICY_H_

#include <cstddef>

namespace memory {

/**
 * @class   AllocationPolicy
 * @brief   This is memory allocation policy interface, to be implemented by allocation policy of choice, eg. bump allocator
 */
class AllocationPolicy {
public:
    AllocationPolicy(size_t memory_start, size_t memory_end);
    virtual ~AllocationPolicy() = default;

    virtual void* alloc(size_t size) = 0;
    virtual void free(void* address) = 0;
    virtual size_t free_memory_in_bytes() = 0;
    virtual size_t total_memory_in_bytes() = 0;

//    virtual void set_low_limit_in_bytes(size_t memory_start);
    virtual void set_high_limit_in_bytes(size_t memory_end);

protected:
    size_t available_memory_first_byte  = 0;    // first available byte for allocation
    size_t available_memory_last_byte   = 0;    // last byte available for allocation
};

} /* namespace memory */

#endif /* SRC_MEMORY_ALLOCATIONPOLICY_H_ */
