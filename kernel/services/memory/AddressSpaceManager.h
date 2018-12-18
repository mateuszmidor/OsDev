/**
 *   @file: AddressSpaceManager.h
 *
 *   @date: Dec 15, 2018
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_ADDRESSSPACEMANAGER_H_
#define SRC_MEMORY_ADDRESSSPACEMANAGER_H_

#include "AddressSpace.h"

namespace memory {

void* alloc_static(AddressSpace& as, size_t size);
void* alloc_stack_and_mark_guard_page(AddressSpace& as, size_t num_bytes);
void release_address_space(AddressSpace& as);

}

#endif