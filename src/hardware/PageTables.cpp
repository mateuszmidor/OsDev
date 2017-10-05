/**
 *   @file: PageTables.cpp
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#include "PageTables.h"
#include "../memory/HigherHalf.h"

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

} /* namespace hardware */
