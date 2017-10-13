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

using utils::KernelLog;
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
        "PROTECTION_VIOLATION"
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
    auto current = mngr.get_current_task();

    u64 faulty_address = get_faulty_address();
    u64* faulty_page = PageTables::get_page_for_virt_address(faulty_address, cpu_state->cr3);// current->pml4_phys_addr);

    // check if PageFault caused by Page Non Present
    PageFaultActualReason pf_reason = page_fault_reason(*faulty_page, cpu_state->error_code);
    if (pf_reason != PageFaultActualReason::PAGE_NOT_PRESENT) {
//        klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". %. KILLING] ",
//                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, current->name.c_str(), PF_REASON[pf_reason]);
        return mngr.kill_current_task();
    }

    // try alloc the page
    if (!alloc_page(faulty_address, faulty_page)) {
//        klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". Could not alloc frame. KILLING] ",
//                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, current->name.c_str());
        return mngr.kill_current_task();
    }

    // resume task execution from where the exception occurred
    return cpu_state;
}

/**
 * @brief   Extract the virtual address that caused PageFault
 */
size_t PageFaultHandler::get_faulty_address() {
    size_t faulty_address;
    asm("mov %%cr2, %%rax;  mov %%rax, %0" : "=m" (faulty_address));
    return faulty_address;
}

/**
 * @brief   Check if given page should be allocated
 * @param   violated_page PageTable entry with lower 12 bits describing the page flags
 * @param   error_code Error code that came with PageFault exception
 * @return  True if the violated_page should be allocated, False if some sort of access violation caused the PageFault
 */
PageFaultActualReason PageFaultHandler::page_fault_reason(u64 violated_page, u64 error_code) const {
    // if PageFault caused by page not present - simply return true
    bool page_not_present = (error_code & PageFaultErrorCode::PRESENT) != PageFaultErrorCode::PRESENT;
    if (page_not_present)
        return PageFaultActualReason::PAGE_NOT_PRESENT;

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

bool PageFaultHandler::alloc_page(size_t virtual_address, u64* page_virt_addr) const {
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();

    size_t frame_phys_addr = memory::FrameAllocator::alloc_frame();
    if (frame_phys_addr == 0)
        return false;

    *page_virt_addr =  frame_phys_addr | PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::USER_ACCESSIBLE | PageAttr::HUGE_PAGE;

    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    return true;
}

} /* namespace cpuexceptions */
