/**
 *   @file: PageFaultHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "PageFaultHandler.h"
#include "PageFaultActualReason.h"
#include "Requests.h"

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
 *          - logging proper information and killing the task if fault caused by protection violation or memory exhaustion
 */
CpuState* PageFaultHandler::on_exception(CpuState* cpu_state) {
    u64 pml4_phys_addr = cpu_state->cr3;
    u64 faulty_address = get_faulty_address();
    PageFaultActualReason pf_reason = requests->get_page_fault_reason(faulty_address, pml4_phys_addr, cpu_state->error_code);

    // check if PageFault caused by Page Non Present
    if (pf_reason != PageFaultActualReason::PAGE_NOT_PRESENT) {
        requests->log(" [PAGE FAULT at % (%MB, %GB) by \"%\". %. KILLING] \n",
                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, requests->get_current_task_name().c_str(), PF_REASON[pf_reason]);
        return requests->kill_current_task_group();
    }

    // try alloc the page
    if (!requests->alloc_missing_page(faulty_address, pml4_phys_addr)) {
    	requests->log(" [PAGE FAULT at % (%MB, %GB) by \"%\". Could not alloc frame. KILLING] \n",
                    faulty_address, faulty_address /1024 / 1024, faulty_address /1024 / 1024 / 1024, requests->get_current_task_name().c_str());
        return requests->kill_current_task_group();
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



} /* namespace cpuexceptions */
