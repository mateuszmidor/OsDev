/**
 *   @file: AllocationPolicy.cpp
 *
 *   @date: Sep 5, 2017
 * @author: Mateusz Midor
 */

#include "AllocationPolicy.h"

namespace memory {

/**
 * Constructor.
 * @brief   Set contiguous memory address range which is available for allocation
 * @param   first_byte  The first byte available for allocation
 * @param   last_byte   The last byte available for allocation
 */
AllocationPolicy::AllocationPolicy(size_t first_byte, size_t last_byte) {
    available_memory_first_byte = first_byte;
    available_memory_last_byte = last_byte;
}

} /* namespace memory */
