/**
 *   @file: HigherHalf.h
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_HIGHERHALF_H_
#define SRC_MEMORY_HIGHERHALF_H_

#include <cstddef>


namespace memory {

/**
 * @brief   This class provides phys-to-virt and virt-to-phys memory address conversion methods for higher half kernel
 */
class HigherHalf {
public:
    /**
     * @brief   Convert kernel virtual address to its corresponding physical address
     */
    template <class T>
    static size_t virt_to_phys(T virtual_addr) {
        return (size_t)virtual_addr - get_kernel_offset();
    }

    /**
     * @brief   Convert physical address to kernel virtual address
     */
    template <class T>
    static size_t phys_to_virt(T physical_addr) {
        return (size_t)physical_addr + get_kernel_offset();
    }

private:
    static size_t get_kernel_offset();
};
}

#endif /* SRC_MEMORY_HIGHERHALF_H_ */
