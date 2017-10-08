/**
 *   @file: PageTables.h
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_HARDWARE_PAGETABLES_H_
#define SRC_HARDWARE_PAGETABLES_H_

#include "types.h"

namespace hardware {

/**
 * @brief   Page attributes that reside in lower 12 bits of page physical address
 *          Lower 12 bits are used since a page is at least 4096(1^12) aligned, so the 12 bits are unused
 * @see     http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, 5.4.1 Field Definitions
 */
enum PageAttr {
    PRESENT         = 1,    // page is mapped to physical address
    WRITABLE        = 2,    // page can be read/written
    USER_ACCESSIBLE = 4,    // page can be accessed from protection ring 3 (user space)
    HUGE_PAGE       = 128,  // page is 2MB (if used in pde) or 1GB (if used in pdpt) instead of standard 4096 bytes
};

struct PageTables64 {
    u64  pml4[512];         // Page Map Level 4
    u64  pdpt[512];         // Page Directory Pointer Table
    u64  pde_kernel[512];   // Page Directory Entry for 2MB pages; map kernel
    u64  pde_user[512];     // Page Directory Entry for 2MB pages; map elf
};

/**
 * @brief   This class configures paging for kernel
 */
class PageTables {
public:
    static void remap_kernel_higher_half();
    static void map_elf_address_space_at(char* phys_addr, size_t num_bytes);

private:
    static u64  pml4[];     // Page Map Level 4
    static u64  pdpt[];     // Page Directory Pointer Table
    static u64  pde[];      // Page Directory Entry
    static u64  pte[];      // Page Table Entry
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_PAGETABLES_H_ */
