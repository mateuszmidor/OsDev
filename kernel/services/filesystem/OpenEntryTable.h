/**
 *   @file: OpenEntryTable.h
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_
#define KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_

#include "Vector.h"
#include "Optional.h"
#include "KernelLog.h"
#include "EntryState.h"
#include "SyscallResult.h"
#include "VfsCachedEntry.h"

namespace filesystem {

/**
 * @brief   Represents a file descriptor that is global throughout the system
 */
using GlobalFileDescriptor = u32;

/**
 * @brief   This struct holds VFS Entry and it's individual open instance state
 */
struct OpenEntry {
    VfsCachedEntryPtr   entry   {nullptr};
    EntryState*         state   {nullptr};
};

class OpenEntryTable {
public:
    OpenEntryTable() : klog(logging::KernelLog::instance()) { }
    void install();
    utils::SyscallResult<GlobalFileDescriptor> open(const VfsCachedEntryPtr& e);
    utils::SyscallResult<VfsCachedEntryPtr> close(GlobalFileDescriptor fd);
    bool is_open(GlobalFileDescriptor fd) const;
    OpenEntry& operator[](GlobalFileDescriptor fd);
    const OpenEntry& operator[](GlobalFileDescriptor fd) const;
private:
    cstd::Optional<GlobalFileDescriptor> find_free_fd() const;
    cstd::vector<OpenEntry> open_entries;
    logging::KernelLog& klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_ */
