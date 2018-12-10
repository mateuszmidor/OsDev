#ifndef COMMON_STUCTS
#define COMMON_STUCTS

#include "List.h"

namespace multitasking {
    class Task;
}

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