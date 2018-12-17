#ifndef COMMON_STUCTS
#define COMMON_STUCTS

#include "List.h"
#include "SyscallResult.h"
#include <memory>
#include <functional>

namespace multitasking {
class Task;
}

namespace memory {
struct AddressSpace {
    u64     heap_low_limit;     // last address allocated for the heap, current program break
    u64     heap_high_limit;    // last address allocable for the heap
    u64     pml4_phys_addr;     // page table root physical address
};
}

namespace filesystem {
/**
 * @brief   Types of entries in virtual filesystem; note that Mountpoint is an extended Directory
 */
enum class VfsEntryType {
    INVALID,
    FILE,
    PIPE,
    DIRECTORY,
};

class VfsEntry;
using VfsEntryPtr = std::shared_ptr<VfsEntry>;
using OnVfsEntryFound = std::function<bool(const VfsEntryPtr& e)>; // for directory contents enumeration. return false to stop the enumeration.


class OpenEntry {
public:
    virtual ~OpenEntry() = default;

        // [common interface]
    virtual VfsEntryType get_type() const  = 0;                                          

    // [file interface]
    virtual utils::SyscallResult<u64> get_size() const  = 0;                             
    virtual utils::SyscallResult<u64> read(void* data, u32 count)  = 0;                  
    virtual utils::SyscallResult<u64> write(const void* data, u32 count)  = 0;           
    virtual utils::SyscallResult<void> seek(u32 new_position)  = 0;                      
    virtual utils::SyscallResult<void> truncate(u32 new_size)  = 0;                      
    virtual utils::SyscallResult<u64> get_position() const  = 0;                         

    // [directory interface]
    virtual utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) = 0;

};

using OpenEntryPtr = std::shared_ptr<OpenEntry>;
} // filesystem

/**
 * @brief   This class represents a list of Tasks to hold the running/waiting tasks
 */
namespace multitasking {
class TaskList : public cstd::List<multitasking::Task*> {
};
}

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