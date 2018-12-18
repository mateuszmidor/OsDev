/**
 *   @file: OpenEntry.h
 *
 *   @date: Dec 18, 2018
 * @author: Mateusz Midor
 */

#ifndef OPEN_ENTRY
#define OPEN_ENTRY

#include "VfsEntry.h"


namespace filesystem {


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

#endif