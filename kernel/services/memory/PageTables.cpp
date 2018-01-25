/**
 *   @file: PageTables.cpp
 *
 *   @date: Oct 5, 2017
 * @author: Mateusz Midor
 */

#include <new>
#include "kstd.h"
#include "PageTables.h"
#include "HigherHalf.h"

namespace memory {

PageTables64  PageTables::kernel_page_tables  __attribute__ ((aligned (4096)));

/**
 * @brief   Map the kernel -2GB virtual memory address space at physical address 0 (where it already is loaded by bootloader)
 *          After this, the first 1GB identity mapping is no longer available and accessing any physical address will end up with page fault
 * @note    Right now only the single -2GB is mapped, -1GB stays unmapped as there is no need for so much virtual address space
 *          Page hierarchy is as follows:
 *          cr3 -> pml4
 *                  -pdpt
 *                     -pde
 *                       -pte (not used when 2MB HUGE pages are used)
 */
void PageTables::map_and_load_kernel_address_space() {
    prepare_higher_half_kernel_page_tables(kernel_page_tables);
    u64 pml4_physical_address = HigherHalf::virt_to_phys(kernel_page_tables.pml4);
    load_address_space(pml4_physical_address);
}

/**
 * @brief   Prepare virtual memory mapping for user 0..1GB and kernel -2GB..0 of virtual memory
 * @param   pml4_phys_addr Physical address of the already allocated PageTables64
 */
void PageTables::map_elf_address_space(size_t pml4_phys_addr) {
    // we need to use kernel virtual addresses for configuring the pages, since no lower 1gb identity mapping exists
    size_t page_tables_virt_addr = HigherHalf::phys_to_virt(pml4_phys_addr);

    // alloc page tables at 4k aligned virtual address
    PageTables64* pt = new ((void*)page_tables_virt_addr)PageTables64;

    // zero the page tables
    memset(pt, 0, sizeof(PageTables64));

    // map user elf virtual address space at 0..1GB and kernel at -2GB..0GB
    prepare_elf_page_tables(*pt);
}

/**
 * @brief   Setup page table entry for "virtual_address" to be a stack guard,
 *          so accessing it can be recognized as stack overflow when page fault occurs
 * @param   virtual_address Address that maps to the page supposed to be used as stack guard page
 */
void PageTables::map_stack_guard_page(size_t virtual_address, size_t pml4_phys_addr) {
    if (u64* page = get_page_for_virt_address(virtual_address, pml4_phys_addr)) {
        *page = PageAttr::STACK_GUARD_PAGE ;
    }
}

/**
 * @brief   Fill PageTables64 with mapping of -2..-1GB virt addresses to 0..1GB phys addresses
 * @note    Kernel memory is not accessible from user space
 */
void PageTables::prepare_higher_half_kernel_page_tables(PageTables64& pt) {
    const u16 PRESENT_WRITABLE = PageAttr::PRESENT | PageAttr::WRITABLE;
    const u16 PRESENT_WRITABLE_HUGE = PRESENT_WRITABLE | PageAttr::HUGE_PAGE;
    // prepare kernel virtual address space in -2..0GB.
    pt.pml4[511] = HigherHalf::virt_to_phys(pt.pdpt)                | PRESENT_WRITABLE;         // last 512 GB chunk
    pt.pdpt[510] = HigherHalf::virt_to_phys(pt.pde_kernel_static)   | PRESENT_WRITABLE;         // -2BG..-1GB chunk
    pt.pdpt[511] = HigherHalf::virt_to_phys(pt.pde_kernel_dynamic)  | PRESENT_WRITABLE;         // -1GB..0GB chunk
    for (u16 i = 0; i < 512; i++)
        pt.pde_kernel_static[i] = (0x200000 * i)                    | PRESENT_WRITABLE_HUGE;    // map static memory pages
}

/**
 * @brief   Fill PageTables64 pml4 and pdpt tables with mapping of lower 1GB virtual memory
 */
void PageTables::prepare_elf_page_tables(PageTables64& pt) {
    const u16 PRESENT_WRITABLE = PageAttr::PRESENT | PageAttr::WRITABLE;
    const u16 PRESENT_WRITABLE_USERSPACE = PRESENT_WRITABLE | PageAttr::USER_ACCESSIBLE;

    // prepare elf virtual address space of 0..1GB as user accessible
    pt.pml4[0] = HigherHalf::virt_to_phys(pt.pdpt)                                  | PRESENT_WRITABLE_USERSPACE;
    pt.pdpt[0] = HigherHalf::virt_to_phys(pt.pde_user)                              | PRESENT_WRITABLE_USERSPACE;

    // prepare kernel virtual address space in -2..0GB. This is necessary so syscall handlers can use kernel addresses
    pt.pml4[511] = HigherHalf::virt_to_phys(kernel_page_tables.pdpt)                | PRESENT_WRITABLE;     // last 512 GB chunk
    pt.pdpt[510] = HigherHalf::virt_to_phys(kernel_page_tables.pde_kernel_static)   | PRESENT_WRITABLE;     // -2BG..-1GB chunk
    pt.pdpt[511] = HigherHalf::virt_to_phys(kernel_page_tables.pde_kernel_dynamic)  | PRESENT_WRITABLE;     // -1GB..0GB chunk
    // specific frame allocation in pde_user for the lower 1GB will happen in PageFault handler
}

/**
 * @brief   Get kernel page tables root physical address
 */
size_t PageTables::get_kernel_pml4_phys_addr() {
    return HigherHalf::virt_to_phys(kernel_page_tables.pml4);
}

/**
 * @brief   Load pml4_physical_address(page tables root) into cr3, effectively setting new memory address space
 */
void PageTables::load_address_space(size_t pml4_physical_address) {
    asm volatile (
            "mov %%rax, %%cr3       ;"
            :
            : "a"(pml4_physical_address)
            :
    );
}

/**
 * @brief   Get Kernel space virtual address of page in "pml4_phys_addr" address space that contains "virtual_address"
 *          or nullptr if "virtual_address" is outside of the address space
 */
u64* PageTables::get_page_for_virt_address(size_t virtual_address, size_t pml4_phys_addr) {
    u16 pml4_index = (virtual_address >> 39) & 511;
    u16 pdpt_index = (virtual_address >> 30) & 511;
    u16 pde_index = (virtual_address >> 21) & 511;

    if (pml4_phys_addr == 0)
        return nullptr;
    u64* pml4_virt_addr = (u64*)HigherHalf::phys_to_virt(pml4_phys_addr);

    if (pml4_virt_addr[pml4_index] == 0)
        return nullptr;
    u64* pdpt_virt_addr = (u64*)HigherHalf::phys_to_virt(pml4_virt_addr[pml4_index] & ~4095); // & ~4095 to remove page flags

    if (pdpt_virt_addr[pdpt_index] == 0)
        return nullptr;
    u64* pde_virt_addr =  (u64*)HigherHalf::phys_to_virt(pdpt_virt_addr[pdpt_index] & ~4095); // & ~4095 to remove page flags

    return &pde_virt_addr[pde_index];
}

/**
 * @brief   Return number of memory pages needed to accomodate num_bytes
 */
size_t PageTables::bytes_to_pages(size_t num_bytes) {
    return (num_bytes / get_page_size()) + 1;
}

} /* namespace hardware */
