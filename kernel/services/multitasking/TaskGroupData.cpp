/**
 *   @file: TaskGroupData.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskGroupData.h"
#include "Requests.h"
#include "PageTables.h"
#include "HigherHalf.h"
#include "MemoryManager.h"

using namespace memory;
namespace multitasking {

TaskGroupData::TaskGroupData(u64 pml4_phys_addr, const cstd::string& cwd, size_t heap_low_limit, size_t heap_high_limit, u32 parent_id) :
        pml4_phys_addr(pml4_phys_addr), cwd(cwd), heap_low_limit(heap_low_limit), heap_high_limit(heap_high_limit), parent_task_id(parent_id) {
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
    // files closed automatically on "files" array destruction
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
    memory::MemoryManager& mngr = memory::MemoryManager::instance();
    requests->log("Removing address space\n");
    for (u32 i = 0; i < 512; i++)   // scan 512 * 2MB pages
        if ((pde_virt_addr[i] & PageAttr::PRESENT) == PageAttr::PRESENT) {
        	requests->log("  Releasing mem frame: %\n", pde_virt_addr[i] / FrameAllocator::get_frame_size());
            mngr.free_frames((void*)pde_virt_addr[i], 1); // 1 byte will always correspond to just 1 frame
        }

    // release the page table itself
    requests->log("  Releasing mem frames of PageTables64: %\n\n", pml4_phys_addr / FrameAllocator::get_frame_size());
    mngr.free_frames((void*)pml4_phys_addr, sizeof(PageTables64));
}

/**
 * @brief   Allocate memory from the top of the heap
 * @note    This memory will not be released until the entire address space is released
 */
void* TaskGroupData::alloc_static(size_t size) {
    // check for out of memory
    if (heap_high_limit - size < heap_low_limit)
        return nullptr;

    heap_high_limit -= size;
    return (void*)heap_high_limit;
}

/**
 * @brief   Allocate memory from the top of the heap for a stack and mark a page below as stack guard page
 * @note    It will actually allocate a multiplicity of PAGE_SIZE that can accommodate "num_bytes"
 * @note    This memory will not be released until the entire address space is released
 */
void* TaskGroupData::alloc_stack_and_mark_guard_page(size_t num_bytes) {
    const auto PAGE_SIZE    = PageTables::get_page_size();

    // stack bottom is page aligned
    auto stack_bottom_page_aligned = (heap_high_limit - num_bytes) & (~(PAGE_SIZE-1));

    // guard page is right below the stack
    auto guard_bottom_page_aligned = stack_bottom_page_aligned - PAGE_SIZE;

    // first usable byte is right below the guard page
    auto new_heap_high_limit = guard_bottom_page_aligned - 1;

    // check for out of memory
    if (new_heap_high_limit < heap_low_limit)
        return nullptr;

    // setup guard page
    PageTables::map_stack_guard_page(guard_bottom_page_aligned, pml4_phys_addr);

    heap_high_limit = new_heap_high_limit;
    return (void*)stack_bottom_page_aligned;
}
} /* namespace multitasking */
