/**
 *   @file: AllocationPolicy.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_ALLOCATIONPOLICY_H_
#define SRC_MEMORY_ALLOCATIONPOLICY_H_

#include "types.h"

namespace memory {

/**
 * @class   AllocationPolicy
 * @brief   This is virtual memory allocation policy interface, to be implemented by allocation policy of choice, eg. bump allocator
 */
class AllocationPolicy {
public:
    AllocationPolicy(size_t memory_start, size_t memory_end);   // virtual memory range to be used
    virtual ~AllocationPolicy() = default;

    // alloc a block of virtual memory and return its address;
    // actual physical memory frames will be allocated by PageFaultHandler; thus the memory is not guaranteed to by physically continuous
    virtual void* alloc_bytes(size_t size) = 0;

    // free a block of virtual memory
    virtual void free_bytes(void* address) = 0;


    // should be moved directly to MemoryManager?
    virtual void* alloc_frames(size_t size) = 0;    // return frame-size aligned physical memory address; contiguous frames are allocated here
    virtual void free_frames(void* address, size_t size) = 0;    // corresponds to alloc_frames, "size" - how many bytes the frames hold

    // should be moved directly to MemoryManager?
    virtual size_t free_memory_in_bytes() = 0;
    virtual size_t total_memory_in_bytes() = 0;

protected:
    size_t available_memory_first_byte  = 0;    // first byte of physical memory available for allocation
    size_t available_memory_last_byte   = 0;    // last byte of physical memory available for allocation
};

} /* namespace memory */

#endif /* SRC_MEMORY_ALLOCATIONPOLICY_H_ */
