/**
 *   @file: HigherHalf.cpp
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#include "HigherHalf.h"

/**
 * @brief   Physical to virtual memory address offset where the kernel is mapped:
 *          virt_address = phys_address + KERNEL_VIRTUAL_BASE
 * @see     boot.S
 */
extern size_t KERNEL_VIRTUAL_BASE;

size_t memory::HigherHalf::get_kernel_offset() {
    return KERNEL_VIRTUAL_BASE;
}
