/**
 *   @file: AddressSpaceManager.h
 *
 *   @date: Dec 15, 2018
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_ADDRESSSPACEMANAGER_H_
#define SRC_MEMORY_ADDRESSSPACEMANAGER_H_

#include "AddressSpace.h"
#include "SyscallResult.h"

namespace memory {

utils::SyscallResult<AddressSpace> alloc_address_space(u64 heap_low_limit, u64 heap_high_limit);
void release_address_space(AddressSpace& as);
void* alloc_static(AddressSpace& as, size_t size);
void* alloc_stack_and_mark_guard_page(AddressSpace& as, size_t num_bytes);

}

#endif
