/**
 *   @file: Allocator.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "AllocationPolicy.h"

namespace memory {

/**
 * Constructor.
 * @brief   Set contiguous memory address range which is available for allocation
 * @note    memory_start is the first byte available for allocation
 *          memory_end is the last byte available for allocation
 */
AllocationPolicy::AllocationPolicy(size_t first_byte, size_t last_byte) {
    available_memory_first_byte = first_byte;
    available_memory_last_byte = last_byte;
}

/**
 * @brief   Set the first byte from which the memory should be allocated
 */
//void AllocationPolicy::set_low_limit_in_bytes(size_t memory_start) {
//    available_memory_start = memory_start;
//}

/**
 * @brief   Set the last byte up to which the memory should be allocated
 */
void AllocationPolicy::set_high_limit_in_bytes(size_t last_byte) {
    available_memory_last_byte = last_byte;
}
} /* namespace memory */
