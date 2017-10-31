/**
 *   @file: PageTables.h
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_HARDWARE_PAGETABLES_H_
#define SRC_HARDWARE_PAGETABLES_H_

#include "types.h"

namespace memory {

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
    u64  pde_kernel_static[512];    // Page Directory Entry for 2MB pages; identity map kernel [-2GB..-1GB] virt -> [0GB..1GB] phys
    u64  pde_kernel_dynamic[512];   // Page Directory Entry for 2MB pages; map kernel -1GB..0GB -> mapped by PageFaultHandler
    u64  pde_user[512];     // Page Directory Entry for 2MB pages; identity map elf [0GB..1GB] virt -> [0GB..1GB] phys
};

/**
 * @brief   This class configures paging for kernel
 */
class PageTables {
public:
    static void map_and_load_kernel_address_space_at_memory_start();
    static void map_elf_address_space(size_t pml4_phys_addr);
    static size_t get_kernel_pml4_phys_addr();
    static void load_address_space(size_t pml4_physical_address);
    static u64* get_page_for_virt_address(size_t virtual_address, size_t pml4_phys_addr);

private:
    static PageTables64 kernel_page_tables;

    static void prepare_higher_half_kernel_page_tables(PageTables64& pt);
    static void prepare_elf_page_tables(PageTables64& pt);
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_PAGETABLES_H_ */
