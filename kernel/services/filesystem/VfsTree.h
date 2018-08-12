/**
 *   @file: VfsTree.h
 *
 *   @date: Jul 23, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_


#include "OpenEntryTable.h"
#include "KernelLog.h"
#include "SyscallResult.h"
#include "EntryCache.h"

namespace filesystem {

struct MountpointPath;

/**
 * @brief   This class represents a virtual file system tree that can be extended by attaching entries like mountpoints and fifos to it.
 *          An entry is stored either as:
 *          1. attachment to the root or one of it's child entries, it is called "attached entry" here.
 *             It may but not need to be in cache(open entries, entries with own attachments are in cache)
 *          2. child of attached entry eg. in case of mountpoint that manages entire filesystem subtree,
 *              it is called "persistent entry" here
 *          Only the root "/" is not attached to anything but just stays cached under "/" path
 */
class VfsTree {
public:
    VfsTree() : klog(logging::KernelLog::instance()) { }
    void install();

    utils::SyscallResult<void> attach(const VfsEntryPtr& entry, const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> create(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> remove(const UnixPath& path);
    utils::SyscallResult<void> copy(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<GlobalFileDescriptor> open(const UnixPath& path);
    utils::SyscallResult<void> close(GlobalFileDescriptor fd);
    bool exists(const UnixPath& path) const;
    OpenEntry& get_entry(GlobalFileDescriptor fd) { return open_entry_table[fd]; }
    const OpenEntry& get_entry(GlobalFileDescriptor fd) const { return open_entry_table[fd]; }
private:
    utils::SyscallResult<void> try_unattach(const UnixPath& path);
    utils::SyscallResult<void> try_uncreate(const UnixPath& path);
    VfsCachedEntryPtr get_or_bring_entry_to_cache(const UnixPath& path);
    void uncache_if_unused(const VfsCachedEntryPtr& e);
    VfsCachedEntryPtr lookup_attached_entry(const UnixPath& path) const;
    VfsEntryPtr lookup_entry(const UnixPath& path) const;
    MountpointPath get_mountpoint_path(const cstd::string& path);

    utils::SyscallResult<void> move_attached_entry(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move_persistent_entry(const UnixPath& path_from, const UnixPath& path_to);

    EntryCache          entry_cache;
    OpenEntryTable      open_entry_table;
    logging::KernelLog& klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_ */
