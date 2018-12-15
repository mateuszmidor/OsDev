#ifndef COMMON_STUCTS
#define COMMON_STUCTS

#include "List.h"

namespace multitasking {
    class Task;
}

struct AddressSpace {
    u64     heap_low_limit;     // last address allocated for the heap, current program break
    u64     heap_high_limit;    // last address allocable for the heap
    u64     pml4_phys_addr;     // page table root physical address
};

/**
 * @brief   This class represents a list of Tasks to hold the running/waiting tasks
 */
class TaskList : public cstd::List<multitasking::Task*> {
};

enum PageFaultActualReason {
    PAGE_NOT_PRESENT,
    READONLY_VIOLATION,
    PRIVILEGE_VIOLATION,
    RESERVED_WRITE_VIOLATION,
    INSTRUCTION_FETCH,
    STACK_OVERFLOW,
    UNKNOWN_PROTECTION_VIOLATION,
    INVALID_ADDRESS_SPACE
};


#endif