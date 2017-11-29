/**
 *   @file: TaskGroupData.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskGroupData.h"
#include "PageTables.h"
#include "HigherHalf.h"
#include "MemoryManager.h"
#include "KernelLog.h"

using namespace memory;
namespace multitasking {

TaskGroupData::TaskGroupData(u64 pml4_phys_addr, const kstd::string& cwd, size_t heap_low_limit, size_t heap_high_limit) :
        pml4_phys_addr(pml4_phys_addr), cwd(cwd), heap_low_limit(heap_low_limit), heap_high_limit(heap_high_limit) {
}

TaskGroupData::~TaskGroupData() {
    close_files();
    release_address_space();
}

/**
 * @brief   Close all the open files in task group
 * @note    Execution context: Interrupt only (on kill_current_task, when Task is destroyed)
 */
void TaskGroupData::close_files() {
    for (auto& f : files)
        if (f)
            f->close();
}

/**
 * @brief   Release physical memory frames allocated for task group
 * @note    Execution context: Interrupt only (on kill_current_task, when Task is destroyed)
 */
void TaskGroupData::release_address_space() {
    if (pml4_phys_addr == 0)
        return;

    u64* pde_virt_addr =  PageTables::get_page_for_virt_address(0, pml4_phys_addr); // user task virtual address space starts at virt address 0

    // scan 0..1GB of virtual memory
    logging::KernelLog& klog = logging::KernelLog::instance();
    memory::MemoryManager& mngr = memory::MemoryManager::instance();
    klog.format("Rmoving address space\n");
    for (u32 i = 0; i < 512; i++)   // scan 512 * 2MB pages
        if (pde_virt_addr[i] != 0) {
            klog.format("  Releasing mem frame: %\n", pde_virt_addr[i] / FrameAllocator::get_frame_size());
            mngr.free_frames((void*)pde_virt_addr[i], 1); // 1 byte will always correspond to just 1 frame
        }

    // release the page table itself
    klog.format("  Releasing mem frames of PageTables64: %\n", pml4_phys_addr / FrameAllocator::get_frame_size());
    mngr.free_frames((void*)pml4_phys_addr, sizeof(PageTables64));
}

/**
 * @brief   Allocate memory from the top of the heap; this memory will never be released.
 *          This is useful eg. for allocating task stack memory
 */
void* TaskGroupData::alloc_static(size_t size) {
    heap_high_limit -= size;
    return (void*)heap_high_limit;
}

} /* namespace multitasking */
