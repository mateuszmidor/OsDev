/**
 *   @file: OpenEntry.h
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_

#include "VfsCachedEntry.h"
#include "EntryState.h"

namespace filesystem {

/**
 * @brief   This struct holds VFS Entry and it's individual open instance state
 */

class OpenEntry;
using OpenEntryPtr = std::unique_ptr<OpenEntry>;

class OpenEntry {
public:
    OpenEntry(VfsCachedEntryPtr e = {}, EntryState* s = {}) : entry(e), state(s) {}
    ~OpenEntry();
    OpenEntry operator=(const OpenEntry&) = delete;
//    OpenEntry(const OpenEntry&) = delete;

    static utils::SyscallResult<OpenEntryPtr> open(const UnixPath& path);
    utils::SyscallResult<u64> get_size() const                                      { return entry->get_size();         }
    utils::SyscallResult<u64> read(void* data, u32 count)                           { return entry->read(data, count);  }
public:
    VfsCachedEntryPtr   entry   {nullptr};
    EntryState*         state   {nullptr};

private:

};



} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_ */
