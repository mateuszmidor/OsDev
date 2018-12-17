#ifndef PAGE_FAULT_H
#define PAGE_FAULT_H

#include "HigherHalf.h"
#include "PageTables.h"
#include "FrameAllocator.h"
#include "CommonStructs.h"

namespace memory{
namespace PageFault {

/**
 * @brief   Possible error code bitfields that come with PageFault exception
 * @see     http://wiki.osdev.org/Exceptions#Page_Fault, Error code
 */
enum class PageFaultErrorCode {
    PRESENT     = 1,    // When set, the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page.
    WRITE       = 2,    // When set, the page fault was caused by a page write. When not set, it was caused by a page read.
    USER        = 4,    // When set, the page fault was caused while CPL = 3. This does not necessarily mean that the page fault was a privilege violation.
    RESERVED    = 8,    // When set, the page fault was caused by reading a 1 in a reserved field.
    INSTR_FETCH = 16    // When set, the page fault was caused by an instruction fetch.
};


/**
 * @brief   Helper function for bit flag checking
 */
template <class EnumType>
static bool is_flag_set(u64 bits, EnumType flag) {
    u64 uflag = (u64)flag;
    return (bits & uflag) == uflag;
}

/**
 * @brief   Check and return the reason why page fault has occured
 * @param   faulty_address Virtual addr that caused page fault
 * @param   pml4_phys_addr Page Tables physical address
 * @param   cpu_error_code Error code that came with PageFault exception
 */
static PageFaultActualReason get_page_fault_reason(u64 faulty_address, u64 pml4_phys_addr, u64 cpu_error_code) {
    u64* violated_page_addr = PageTables::get_page_for_virt_address(faulty_address, pml4_phys_addr);
    if (!violated_page_addr)
        return PageFaultActualReason::INVALID_ADDRESS_SPACE;

    u64 violated_page = *violated_page_addr;

    // if PageFault caused by page not present - check if this page is marked as stack guard
    bool page_not_present = !is_flag_set(cpu_error_code, PageFaultErrorCode::PRESENT);
    if (page_not_present) {
        bool stack_guard_page = is_flag_set(violated_page, PageAttr::STACK_GUARD_PAGE);
        if (stack_guard_page)
            return PageFaultActualReason::STACK_OVERFLOW;
        else
            return PageFaultActualReason::PAGE_NOT_PRESENT;
    }

    // if PageFault caused by page-protection violation, examine the actual reason
    bool caused_by_write = is_flag_set(cpu_error_code, PageFaultErrorCode::WRITE);
    bool page_readonly = !is_flag_set(violated_page, PageAttr::WRITABLE);
    bool readonly_violation = caused_by_write && page_readonly;
    if (readonly_violation)
        return PageFaultActualReason::READONLY_VIOLATION;


    bool caused_in_usermode = is_flag_set(cpu_error_code, PageFaultErrorCode::USER);
    bool page_kernel_access_only = !is_flag_set(violated_page, PageAttr::USER_ACCESSIBLE);
    bool privilege_violation = caused_in_usermode && page_kernel_access_only;
    if (privilege_violation)
        return PageFaultActualReason::PRIVILEGE_VIOLATION;

    bool caused_by_reserved = is_flag_set(cpu_error_code, PageFaultErrorCode::RESERVED);
    if (caused_by_reserved)
        return PageFaultActualReason::RESERVED_WRITE_VIOLATION;

    bool caused_by_instrfetch = is_flag_set(cpu_error_code, PageFaultErrorCode::INSTR_FETCH);
    if (caused_by_instrfetch)
        return PageFaultActualReason::INSTRUCTION_FETCH;

    return PageFaultActualReason::UNKNOWN_PROTECTION_VIOLATION;
}

/**
 * @brief   Allocate missing page
 *          Give WRITABLE permission by default.
 *          This can change in future to provide more detailed control eg. for kernel/elf readonly and read/write data sections.
 */
static bool alloc_missing_page(u64 virtual_address, u64 pml4_phys_addr) {
    s64 frame_phys_addr = FrameAllocator::alloc_frame();
    if (frame_phys_addr == -1)
        return false;

    u64* page_virt_addr = PageTables::get_page_for_virt_address(virtual_address, pml4_phys_addr);
    *page_virt_addr = frame_phys_addr | PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::HUGE_PAGE;

    bool is_kernel_address_space = (u64)virtual_address >= HigherHalf::get_kernel_heap_low_limit();
    if (is_kernel_address_space)
        *page_virt_addr |= PageAttr::GLOBAL_PAGE;
    else
        *page_virt_addr |= PageAttr::USER_ACCESSIBLE;

    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    return true;
}

}
}
#endif