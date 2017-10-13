/**
 *   @file: BumpAllocator.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_BUMPALLOCATIONPOLICY_H_
#define SRC_MEMORY_BUMPALLOCATIONPOLICY_H_

#include "AllocationPolicy.h"

namespace memory {

class BumpAllocationPolicy: public AllocationPolicy {
public:
    BumpAllocationPolicy(size_t available_memory_start, size_t available_memory_end);

    void* alloc_bytes(size_t size) override;
    void* alloc_frames(size_t size) override;
    void free_bytes(void* address) override;
    void free_frames(void* address, size_t size) override;
    size_t free_memory_in_bytes() override;
    size_t total_memory_in_bytes() override;

private:
    volatile size_t bump_phys_addr;
};

} /* namespace memory */

#endif /* SRC_MEMORY_BUMPALLOCATIONPOLICY_H_ */
