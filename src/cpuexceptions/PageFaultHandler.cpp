/**
 *   @file: PageFaultHandler.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "PageFaultHandler.h"
#include "KernelLog.h"
#include "TaskManager.h"
#include "HigherHalf.h"
#include "PageTables.h"
#include "FrameAllocator.h"

using utils::KernelLog;
using hardware::CpuState;
using multitasking::TaskManager;
using namespace memory;
using namespace hardware;

namespace cpuexceptions {

s16 PageFaultHandler::handled_exception_no() {
    return hardware::Interrupts::PageFault;
}

/**
 * @brief   Handle page fault by printing message and killing offending task.
 *          In future it should conditionally allocate the page and resume task execution
 */
CpuState* PageFaultHandler::on_exception(CpuState* cpu_state) {
    u64 faulty_address;
    asm volatile("mov %%cr2, %%rax;  mov %%rax, %0" : "=m"(faulty_address));

    KernelLog& klog = KernelLog::instance();
    TaskManager& mngr = TaskManager::instance();

    if (!alloc_frame(faulty_address)) {
        auto current = mngr.get_current_task();
        klog.format(" [PAGE FAULT at % (%MB, %GB) by \"%\". KILLING] ",
                    faulty_address, faulty_address /1024 / 1024,  faulty_address /1024 / 1024 / 1024, current->name.c_str());
        return mngr.kill_current_task();
    }
//
    return cpu_state;
}

bool PageFaultHandler::alloc_frame(size_t virtual_address) {
    TaskManager& mngr = TaskManager::instance();
    auto current = mngr.get_current_task();

    u16 pml4_index = (virtual_address >> 39) & 511;
    u16 pdpt_index = (virtual_address >> 30) & 511;
    u16 pde_index = (virtual_address >> 21) & 511;

    u64* pml4_virt_addr = (u64*)memory::HigherHalf::phys_to_virt(current->pml4_phys_addr);
    u64* pdpt_virt_addr = (u64*)memory::HigherHalf::phys_to_virt(pml4_virt_addr[pml4_index]);
    u64* pde_virt_addr = (u64*)memory::HigherHalf::phys_to_virt(pdpt_virt_addr[pdpt_index]);

    size_t frame_phys_addr = memory::FrameAllocator::alloc_frame();
    if (frame_phys_addr == 0)
        return false;

    pde_virt_addr[pde_index] =  frame_phys_addr | PageAttr::PRESENT | PageAttr::WRITABLE | PageAttr::USER_ACCESSIBLE | PageAttr::HUGE_PAGE;

    //asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    asm volatile (
            "mov %%cr3, %%rax       ;"
            "mov %%rax, %%cr3       ;"
            : : : "%rax");

    return true;
}
} /* namespace cpuexceptions */
