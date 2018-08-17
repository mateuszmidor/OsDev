/**
 *   @file: VfsTree.h
 *
 *   @date: Jul 23, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_


#include "KernelLog.h"
#include "SyscallResult.h"
#include "EntryCache.h"

namespace filesystem {

struct MountpointPath;

/**
 * @brief   This class represents a virtual file system tree that can be extended by attaching entries like mountpoints and fifos to it.
 *          An entry is stored either as:
 *          1. attachment to the root or one of it's child entries, it is called "attached entry" here as it is not managed by any mountpoint.
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
    utils::SyscallResult<void> create(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> remove(const UnixPath& path);
    utils::SyscallResult<void> copy(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move(const UnixPath& path_from, const UnixPath& path_to);
    bool exists(const UnixPath& path) const;
    VfsCachedEntryPtr get_cached(const UnixPath& path) { return get_or_bring_entry_to_cache(path); }
    bool release_cached(const VfsCachedEntryPtr& e) { return uncache_if_unused(e); }

private:
    utils::SyscallResult<void> try_unattach(const UnixPath& path);
    utils::SyscallResult<void> try_uncreate(const UnixPath& path);
    VfsCachedEntryPtr get_or_bring_entry_to_cache(const UnixPath& path);
    bool uncache_if_unused(const VfsCachedEntryPtr& e);
    VfsEntryPtr lookup_attached_entry(const UnixPath& path) const;
    VfsEntryPtr lookup_entry(const UnixPath& path) const;
    MountpointPath get_mountpoint_path(const cstd::string& path);

    utils::SyscallResult<void> move_attached_entry(const UnixPath& path_from, const UnixPath& path_to);
    utils::SyscallResult<void> move_persistent_entry(const UnixPath& path_from, const UnixPath& path_to);

    EntryCache          entry_cache;
    logging::KernelLog& klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_ */
