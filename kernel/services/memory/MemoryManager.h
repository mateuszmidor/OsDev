/**
 *   @file: MemoryManager.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_MEMORYMANAGER_H_
#define SRC_MEMORY_MEMORYMANAGER_H_

#include "HigherHalf.h"
#include "FrameAllocator.h"
#include "AllocationPolicy.h"

namespace memory {

/**
 * @class   MemoryManager
 * @brief   This guy will manage dynamic memory (new/delete) in the system
 */
class MemoryManager {
public:
    static MemoryManager& instance();

    void* alloc_frames(size_t size) const;
    void free_frames(void* address, size_t size) const;

    void* alloc_virt_memory(size_t size) const;
    void free_virt_memory(void* address) const;

    size_t get_free_memory_in_bytes() const;
    size_t get_total_memory_in_bytes() const;

    /**
     * @brief   Setup how the memory manager will handle memory. This must be done before running any other C/C++ code that needs dynamic memory
     * @param   first_byte    First physical byte available for allocation. Set it high enough not to overwrite kernel code/data that start at 1MB
     * @param   last_byte     Last physical byte available for allocation.
     */
    template <typename AllocationPolicyType>
    static void install_allocation_policy(size_t first_phys_byte, size_t last_phys_byte) {
        FrameAllocator::init(first_phys_byte, last_phys_byte);
        static char storage[sizeof(AllocationPolicyType)]; // policy is stored here since no dynamic memory is available yet, but we're just working on it :)

        size_t dynamic_start = HigherHalf::get_kernel_heap_low_limit();
        size_t dynamic_end = dynamic_start + (last_phys_byte - first_phys_byte);
        allocation_policy = new (storage) AllocationPolicyType(dynamic_start, dynamic_end);
    }

private:
    MemoryManager() {};

    static MemoryManager _instance;
    static AllocationPolicy* allocation_policy;
};

} /* namespace memory */

#endif /* SRC_MEMORY_MEMORYMANAGER_H_ */
