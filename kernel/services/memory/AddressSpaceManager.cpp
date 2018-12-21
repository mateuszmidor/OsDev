/**
 *   @file: AddressSpaceManager.cpp
 *
 *   @date: Dec 15, 2018
 * @author: Mateusz Midor
 */

#include "AddressSpaceManager.h"
#include "MemoryManager.h"

using namespace middlespace;
namespace memory {

/**
 * @brief	Allocate and prepare an address space for new task group
 */
utils::SyscallResult<AddressSpace> alloc_address_space(u64 heap_low_limit, u64 heap_high_limit) {
	MemoryManager& mm = MemoryManager::instance();
	u64 pml4_phys_addr = (u64)mm.alloc_frames(sizeof(PageTables64)); // must be physical, continuous address space
	if (pml4_phys_addr == 0)
		return {ErrorCode::EC_NOMEM};

    // create address space mapping for new task group
    PageTables::map_elf_address_space(pml4_phys_addr);

    // return a ready to use address space
    return {{heap_low_limit, heap_high_limit, pml4_phys_addr}};
}

/**
 * @brief   Release physical memory frames allocated for task group address space
 * @note    Execution context: Interrupt only (on kill_current_task, when Task is destroyed)
 */
void release_address_space(AddressSpace& as) {
    if (as.pml4_phys_addr == 0)
        return;

    u64* pde_virt_addr =  PageTables::get_page_for_virt_address(0, as.pml4_phys_addr); // user task virtual address space starts at virt address 0

    // scan 0..1GB of virtual memory
    MemoryManager& mngr = MemoryManager::instance();

    //requests->log("Removing address space\n");
    for (u32 i = 0; i < 512; i++)   // scan 512 * 2MB pages
        if ((pde_virt_addr[i] & PageAttr::PRESENT) == PageAttr::PRESENT) {
        	//requests->log("  Releasing mem frame: %\n", pde_virt_addr[i] / FrameAllocator::get_frame_size());
            mngr.free_frames((void*)pde_virt_addr[i], 1); // 1 byte will always correspond to just 1 frame
        }

    // release the page table itself
    //requests->log("  Releasing mem frames of PageTables64: %\n\n", pml4_phys_addr / FrameAllocator::get_frame_size());
    mngr.free_frames((void*)as.pml4_phys_addr, sizeof(PageTables64));

    // mark address space as invalid
    as.pml4_phys_addr = 0;
}

/**
 * @brief   Allocate memory from the top of the heap and return pointer to that memory
 * @return  nullptr if no memory left, pointer to allocated memory otherwise
 * @note    This memory will not be released until the entire address space is released
 */
void* alloc_static(AddressSpace& as, size_t size) {
    // check for out of memory
    if (as.heap_high_limit - size < as.heap_low_limit)
        return nullptr;

    as.heap_high_limit -= size;
    return (void*)as.heap_high_limit;
}

/**
 * @brief   Allocate memory from the top of the heap for a stack and mark a page below as stack guard page
 * @return  nullptr if no memory left, pointer to allocated stack otherwise
 * @note    It will actually allocate a multiplicity of PAGE_SIZE that can accommodate "num_bytes"
 * @note    This memory will not be released until the entire address space is released
 *          means if a thread dies - we have a memory leak
 */
void* alloc_stack_and_mark_guard_page(AddressSpace& as, size_t num_bytes) {
    constexpr u64 PAGE_SIZE = PageTables::get_page_size();

    // stack bottom is page aligned
    u64 stack_bottom_page_aligned = (as.heap_high_limit - num_bytes) & (~(PAGE_SIZE-1));

    // guard page is right below the stack
    u64 guard_bottom_page_aligned = stack_bottom_page_aligned - PAGE_SIZE;

    // first usable byte is right below the guard page
    u64 new_heap_high_limit = guard_bottom_page_aligned - 1;

    // check for out of memory
    if (new_heap_high_limit < as.heap_low_limit)
        return nullptr;

    // setup guard page
    PageTables::map_stack_guard_page(guard_bottom_page_aligned, as.pml4_phys_addr);

    as.heap_high_limit = new_heap_high_limit;
    return (void*)stack_bottom_page_aligned;
}
}
