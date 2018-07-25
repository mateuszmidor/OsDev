/**
 *   @file: VfsTree.h
 *
 *   @date: Jul 23, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_

#include "VfsCachedEntry.h"
#include "KernelLog.h"
#include "SyscallResult.h"
#include "Map.h"
#include "Optional.h"

namespace filesystem {

using GlobalFileDescriptor = u32;

class VfsTree {
public:
    VfsTree() : klog(logging::KernelLog::instance()) { }

    utils::SyscallResult<void> attach(const VfsEntryPtr& entry, const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> open(const UnixPath& path);
    void close(GlobalFileDescriptor fd);
    bool exists(const UnixPath& path) const;
    void install();

private:
    cstd::Optional<GlobalFileDescriptor> get_fd_for_path(const UnixPath& path) const;
    void set_fd_for_path(GlobalFileDescriptor fd, const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> get_or_bring_entry_to_cache(const UnixPath& path);

    VfsEntryPtr lookup_entry(const UnixPath& path) const;
    cstd::Optional<GlobalFileDescriptor> allocate_entry_in_open_table(const VfsEntryPtr& e);
    cstd::Optional<GlobalFileDescriptor> find_free_fd_in_open_table() const;

    cstd::Map<UnixPath, GlobalFileDescriptor>   path_to_filedescriptor;
    cstd::vector<VfsCachedEntryPtr>             cached_entries;// root is always here
    logging::KernelLog&                         klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_ */
