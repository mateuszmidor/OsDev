/**
 *   @file: MemoryManager.h
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_MEMORYMANAGER_H_
#define SRC_MEMORY_MEMORYMANAGER_H_

#include "BumpAllocationPolicy.h"

namespace memory {

/**
 * @class   MemoryManager
 * @brief   This guy will manage dynamic memory (new/delete) in the system
 */
class MemoryManager {
public:
    static MemoryManager& instance();
    void* alloc(size_t size) const;
    void free(void* address) const;
    void set_high_limit(size_t limit) const;
    size_t get_free_memory_in_bytes() const;
    size_t get_total_memory_in_bytes() const;

    /**
     * @brief   Setup how the memory manager will handle memory. This must be done before running any other C/C++ code that needs dynamic memory
     * @param   first_byte    First byte available for allocation. Set it high enough so multiboot2 nor kernel code/data gets overriden. EG 10MB
     * @param   last_byte     Last byte available for allocation.
     */
    template <class AllocationPolicyType>
    static void install_allocation_policy(size_t first_byte, size_t last_byte) {
        static char storage[sizeof(AllocationPolicyType)]; // policy is stored here since no dynamic memory is available yet, but we're working on it :)
        allocation_policy = new (storage) AllocationPolicyType(first_byte, last_byte);
    }

private:
    MemoryManager() {};

    static MemoryManager _instance;
    static AllocationPolicy* allocation_policy;
};

} /* namespace memory */

#endif /* SRC_MEMORY_MEMORYMANAGER_H_ */
