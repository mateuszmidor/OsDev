/**
 *   @file: PageTables.cpp
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "PageTables.h"
#include "HigherHalf.h"

using namespace memory;
namespace hardware {

u64  PageTables::pml4[512]  __attribute__ ((aligned (4096)));
u64  PageTables::pdpt[512]  __attribute__ ((aligned (4096)));
u64  PageTables::pde[512]   __attribute__ ((aligned (4096)));
u64  PageTables::pte[512]   __attribute__ ((aligned (4096)));

/**
 * @brief   Map the kernel into -2GB of virtual address space
 *          After this, the first 1GB identity mapping is no longer available and accessing any physical address will end up with page fault
 * @note    Right now only the single -2GB is mapped, -1GB stays unmapped as there is no need for so much virtual address space
 *          Page hierarchy is as follows:
 *          cr3 -> pml4
 *                  -pdpt
 *                     -pde
 *                       -pte (not used when 2MB HUGE pages are used)
 */
void PageTables::remap_kernel_higher_half() {
    const u16 PRESENT_WRITABLE_USERSPACE = PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::USER_ACCESSIBLE;

    pml4[511] = HigherHalf::virt_to_phys(pdpt)  | PRESENT_WRITABLE_USERSPACE;
    pdpt[510] = HigherHalf::virt_to_phys(pde)   | PRESENT_WRITABLE_USERSPACE;
    for (u16 i = 0; i < 512; i++)
        pde[i] = (0x200000 * i) | PRESENT_WRITABLE_USERSPACE | PageAttr::HUGE_PAGE;

    u64 pml4_physical_address = HigherHalf::virt_to_phys(pml4);

    asm volatile (
            "mov %%rax, %%cr3       ;"
            :
            : "a"(pml4_physical_address)
            :
    );
}

/**
 * @brief   Map virtual address space 0..num_bytes-1 to physical phys_addr..phys_addr+num_bytes-1
 * @param   phys_addr Where the elf physical memory starts
 * @param   num_bytes How long is the elf physical memory chunk
 * @note    Temporary solution until proper frame allocator and memory manager is developed
 */
u64 PageTables::map_elf_address_space_at(char* phys_addr, size_t num_bytes) {
    // get 4k aligned physical addr to allocate page tables struct
    size_t phys_addr_align4k = ((size_t)phys_addr + 4095) & 0xFFFFFFFFFFFFF000; // align to 4KB

    // we need to use kernel virtual addresses for configuring pages, since no lower 1gb identity mapping exists
    size_t virt_addr_align4k = HigherHalf::phys_to_virt(phys_addr_align4k);

    // alloc page tables at 4k aligned virtual address
    PageTables64* pt = new ((void*)virt_addr_align4k)PageTables64;

    // zero the page tables
    memset(pt, 0, sizeof(PageTables64));

    // user memory starts just after the page tables
    size_t user_phys_address = phys_addr_align4k + sizeof(PageTables64);
    size_t user_phys_address_align2m = (user_phys_address + 2097151) & 0xFFFFFFFFFFE00000; // align to 2MB since huge pages are 2MB aligned

    const u16 PRESENT_WRITABLE_USERSPACE = PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::USER_ACCESSIBLE;

    // map kernel virtual address space at -2GB
    pt->pml4[511] = HigherHalf::virt_to_phys(pt->pdpt)  | PRESENT_WRITABLE_USERSPACE;
    pt->pdpt[510] = HigherHalf::virt_to_phys(pt->pde_kernel)   | PRESENT_WRITABLE_USERSPACE;
    for (u16 i = 0; i < 512; i++)
        pt->pde_kernel[i] = (0x200000 * i) | PRESENT_WRITABLE_USERSPACE | PageAttr::HUGE_PAGE;

    // map user elf virtual address space at 1GB
    size_t num_pages = num_bytes / 0x200000 + 1;
    pt->pml4[0] = HigherHalf::virt_to_phys(pt->pdpt)  | PRESENT_WRITABLE_USERSPACE;
    pt->pdpt[0] = HigherHalf::virt_to_phys(pt->pde_user)   | PRESENT_WRITABLE_USERSPACE;
    for (u16 i = 0; i < num_pages; i++) // later when using 4k pages maybe dont map the nullptr address to catch nullptrs?
        pt->pde_user[i] = (0x200000 * i + user_phys_address_align2m) | PRESENT_WRITABLE_USERSPACE | PageAttr::HUGE_PAGE;

    u64 pml4_physical_address = HigherHalf::virt_to_phys(pt->pml4);

    asm volatile (
            "mov %%rax, %%cr3       ;"
            :
            : "a"(pml4_physical_address)
            :
    );

    return pml4_physical_address;
}

u64 PageTables::get_pml4_phys_addr() {
    return HigherHalf::virt_to_phys(pml4);
}
} /* namespace hardware */
