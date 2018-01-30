/**
 *   @file: PageFaultHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "PageFaultHandler.h"
#include "KernelLog.h"
#include "HigherHalf.h"
#include "PageTables.h"
#include "FrameAllocator.h"
#include "TaskManager.h"

using logging::KernelLog;
using hardware::CpuState;
using multitasking::TaskManager;
using namespace memory;
using namespace hardware;

namespace cpuexceptions {

/**
 * @brief   PageFault reason descriptions. The directly correspond to PageFaultActualReason
*/
const char* PageFaultHandler::PF_REASON[] = {
        "PAGE_NOT_PRESENT",
        "READONLY_VIOLATION",
        "PRIVILEGE_VIOLATION",
        "RESERVED_WRITE_VIOLATION",
        "INSTRUCTION_FETCH",
        "STACK_OVERFLOW",
        "PROTECTION_VIOLATION",
        "INVALID_ADDRESS_SPACE"
};

s16 PageFaultHandler::handled_exception_no() {
    return hardware::Interrupts::PageFault;
}

/**
 * @brief   Handle page fault by either:
 *          - allocating the page if fault caused by page non present
 *          - logging proper information and killing the task if fault caused by protection violation
 */
CpuState* PageFaultHandler::on_exception(CpuState* cpu_state) {
    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();
    multitasking::Task& current = mngr.get_current_task();

    u64 faulty_address = get_faulty_address();
    u64* faulty_page = PageTables::get_page_for_virt_address(faulty_address, cpu_state->cr3);
    PageFaultActualReason pf_reason = page_fault_reason(faulty_page, cpu_state->error_code);

    // check if PageFault caused by Page Non Present
    if (pf_reason != PageFaultActualReason::PAGE_NOT_PRESENT) {
        klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". %. KILLING] \n",
                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, current.name.c_str(), PF_REASON[pf_reason]);
        return mngr.kill_current_task_group();
    }

    // try alloc the page
    if (!alloc_page(faulty_address, faulty_page)) {
        klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". Could not alloc frame. KILLING] \n",
                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, current.name.c_str());
        return mngr.kill_current_task_group();
    }

    // This triggers recursive page faulting, dont use
//    klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". New frame allocated] \n",
//                faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, current.name.c_str());

    // resume task execution from where the exception occurred
    return cpu_state;
}

/**
 * @brief   Extract the virtual address that caused PageFault
 */
size_t PageFaultHandler::get_faulty_address() {
    size_t faulty_address;
    asm("mov %%cr2, %%rax" : "=a" (faulty_address));
    return faulty_address;
}

/**
 * @brief   Check if given page should be allocated
 * @param   violated_page_addr Phys addr of PageTable entry with lower 12 bits describing the page flags
 * @param   error_code Error code that came with PageFault exception
 * @return  True if the violated_page should be allocated, False if some sort of access violation caused the PageFault
 */
PageFaultActualReason PageFaultHandler::page_fault_reason(u64* violated_page_addr, u64 error_code) const {
    if (!violated_page_addr)
        return PageFaultActualReason::INVALID_ADDRESS_SPACE;

    u64 violated_page = *violated_page_addr;

    // if PageFault caused by page not present - check if this page is marked as stack guard
    bool page_not_present = (error_code & PageFaultErrorCode::PRESENT) != PageFaultErrorCode::PRESENT;
    if (page_not_present) {
        bool stack_guard_page = (violated_page & PageAttr::STACK_GUARD_PAGE) == PageAttr::STACK_GUARD_PAGE;
        if (stack_guard_page)
            return PageFaultActualReason::STACK_OVERFLOW;
        else
            return PageFaultActualReason::PAGE_NOT_PRESENT;
    }

    // if PageFault caused by page-protection violation, examine the actual reason
    bool caused_by_write = (error_code & PageFaultErrorCode::WRITE) == PageFaultErrorCode::WRITE;
    bool page_readonly = (violated_page & PageAttr::WRITABLE) != PageAttr::WRITABLE;
    bool readonly_violation = caused_by_write && page_readonly;
    if (readonly_violation)
        return PageFaultActualReason::READONLY_VIOLATION;


    bool caused_in_usermode = (error_code & PageFaultErrorCode::USER) == PageFaultErrorCode::USER;
    bool page_kernel_access_only = (violated_page & PageAttr::USER_ACCESSIBLE) != PageAttr::USER_ACCESSIBLE;
    bool privilege_violation = caused_in_usermode && page_kernel_access_only;
    if (privilege_violation)
        return PageFaultActualReason::PRIVILEGE_VIOLATION;

    bool caused_by_reserved = (error_code & PageFaultErrorCode::RESERVED) == PageFaultErrorCode::RESERVED;
    if (caused_by_reserved)
        return PageFaultActualReason::RESERVED_WRITE_VIOLATION;

    bool caused_by_instrfetch = (error_code & PageFaultErrorCode::INSTR_FETCH) == PageFaultErrorCode::INSTR_FETCH;
    if (caused_by_instrfetch)
        return PageFaultActualReason::INSTRUCTION_FETCH;

    return PageFaultActualReason::UNKNOWN_PROTECTION_VIOLATION;
}

/**
 * @brief   Allocate missing page.
 *          Give WRITABLE permission by default.
 *          This can change in future to provide more detailed control eg. for kernel/elf readonly and read/write data sections.
 */
bool PageFaultHandler::alloc_page(size_t virtual_address, u64* page_virt_addr) const {
    size_t frame_phys_addr = memory::FrameAllocator::alloc_frame();
    if (frame_phys_addr == -1)
        return false;

    *page_virt_addr = frame_phys_addr | PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::HUGE_PAGE;

    bool is_kernel_address_space = (size_t)page_virt_addr >= HigherHalf::get_kernel_heap_low_limit();
    if (is_kernel_address_space)
        *page_virt_addr |= PageAttr::GLOBAL_PAGE;
    else
        *page_virt_addr |= PageAttr::USER_ACCESSIBLE;

    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    return true;
}

} /* namespace cpuexceptions */
