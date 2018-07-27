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

/**
 * @brief   This class represents a virtual file system tree that can be extended by attaching entries like mountpoints and fifos to it
 *          An entry is stored either as
 *          1. directly cached entry in vector
 *          2. attachment to directly cached entry
 *          3. child of directly cached entry eg. in case of mountpoint
 *
 *          On close():
 *              make sure refcount is 0 (no one else has it open)
 *              AND it has no attachments (directory empty),
 *              then delete it from vector and map
 *              if marked remove-pending, remove it from persistent storage?
 *
 *          To remove entry:
 *          1. from cache - make sure refcount is 0 (no one has it open)
 *              AND it has no attachments (directory empty),
 *              then delete it from vector and map
 *
 *          2. from persistent storage - make sure refcount is 0 (no one has it open)
 *              AND it has no attachments (directory empty),
 *              then just delegate removing to mountpoint
 */
class VfsTree {
public:
    VfsTree() : klog(logging::KernelLog::instance()) { }
    void install();

    utils::SyscallResult<void> attach(const VfsEntryPtr& entry, const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> create(const UnixPath& path, bool is_directory);
    utils::SyscallResult<void> remove(const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> open(const UnixPath& path);
    void close(GlobalFileDescriptor fd);
    bool exists(const UnixPath& path) const;

private:
    bool uncache(const UnixPath& path);
    bool uncreate(const UnixPath& path);
    utils::SyscallResult<GlobalFileDescriptor> get_or_bring_entry_to_cache(const UnixPath& path);
    VfsEntryPtr lookup_entry(const UnixPath& path) const;

    EntryCache          entry_cache;
    logging::KernelLog& klog;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSTREE_H_ */
