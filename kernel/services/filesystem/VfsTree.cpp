/**
 *   @file: VfsTree.cpp
 *
 *   @date: Jul 23, 2018
 * @author: Mateusz Midor
 */

#include "VfsTree.h"
#include "StringUtils.h"
#include "VfsCachedEntry.h"
#include "VfsRamDirectoryEntry.h"
#include "List.h"

using namespace cstd;
using namespace middlespace;

namespace filesystem {

/**
 * @brief   This struct holds a mountpoint vfs entry and a path to an element under that mountpoint
 */
struct MountpointPath {
    VfsEntryPtr     mountpoint;
    cstd::string    path;       // path that lives in mountpoint
    operator bool() { return (bool)mountpoint; }
    bool operator!(){ return !mountpoint; }
};

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, nullptr otherwise
 */
static VfsEntryPtr get_entry_in_dir(const string& name, VfsEntryPtr parent_dir) {
    VfsEntryPtr result;
    auto on_entry = [&](VfsEntryPtr e) -> bool {
        if (e->get_name() == name) {
            result = e;
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };

    parent_dir->enumerate_entries(on_entry);
    return result;
}

/**
 * @brief   Cut off the last segment of "path" and return it, "path" itself becomes shorter
 * @note    If last segment is cut, "path" becomes the root "/"
 */
static string snap_path_tail(string& path) {
    string result = StringUtils::snap_tail(path, '/');

    // when last segment has been cut, the root remains
    if (path.empty())
        path = "/";

    return result;
}

/**
 * @brief   Prepare the vfs tree to work with dynamic memory once it is available in system
 */
void VfsTree::install() {
    auto root_dir = std::make_shared<VfsRamDirectoryEntry>("/"); // empty root dir
    entry_cache.install();
    entry_cache.allocate(root_dir, "/");
}

/**
 * @brief   Attach "entry" under given "parent_path", return error code on error
 */
utils::SyscallResult<void> VfsTree::attach(const VfsEntryPtr& entry, const UnixPath& parent_path) {
    // get the parent from depths of virtual file system and cache it so we can attach to it
    auto parent_fd = get_or_bring_entry_to_cache(parent_path);

    if (!parent_fd) {
        klog.format("VfsTree::attach: can't attach to '%s'\n", parent_path);
        return {parent_fd.ec};
    }

    // attach entry to parent
    if (!entry_cache[parent_fd.value]->attach_entry(entry)) {
        klog.format("VfsTree::attach: entry '%s' already exists at '%s'\n", entry->get_name(), parent_path);
        return {ErrorCode::EC_EXIST};
    }

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Create file/directory pointed by "path" and return its file descriptor on success, or error code otherwise
 * @note    The actual entry creation is delegated to a mountpoint on the "path" thus mountpoint is required
 */
utils::SyscallResult<GlobalFileDescriptor> VfsTree::create(const UnixPath& path, bool is_directory) {
    // find the mountpoint responsible for managing the "path"
    auto mp = get_mountpoint_path(path);
    if (!mp) {
        klog.format("VfsTree::create: target located on unmodifiable filesystem: %\n", path);
        return {ErrorCode::EC_ROFS};
    }

    // make it create the entry
    auto result = mp.mountpoint->create_entry(mp.path, is_directory);
    if (!result) {
        klog.format("VfsTree::create: target filesystem refused to create entry: %\n", path);
        return {result.ec};
    }

    // cache the created entry
    auto fd = entry_cache.allocate(result.value, path);
    if (!fd) {
        klog.format("VfsTree::create: open file limit reached\n");
        return {ErrorCode::EC_MFILE};
    }

    // create also opens, and open increases refcount
    entry_cache[fd.value]->refcount++;
    return {fd.value};
}

/**
 * @brief   Delete file or an empty directory pointed by "path"
 *          From persistent (mountpoint) + from cache + from cache attachment, uff
 */
utils::SyscallResult<void> VfsTree::remove(const UnixPath& path) {
    bool uncached {false};

    // check if entry in cache and eligible for removal
    if (VfsCachedEntryPtr e = lookup_cached_entry(path)) {
        if (e->refcount > 0) {
            klog.format("VfsTree::remove: cant remove; entry '%s' is open\n", path);
            return {ErrorCode::EC_ISOPEN};
        }

        if (e->attachment_count() > 0) {
            klog.format("VfsTree::remove: cant remove; entry '%s' not empty\n", path);
            return {ErrorCode::EC_NOTEMPTY};
        }

        // remove from cache
        uncached = uncache(path);
    }

    // remove from persistent storage if eligible
    bool uncreated = uncreate(path);

    // any removal is a success
    if (uncached || uncreated)
        return {ErrorCode::EC_OK};

    return {ErrorCode::EC_NOENT};
}

utils::SyscallResult<void> VfsTree::move_entry(const UnixPath& path_from, const UnixPath& path_to) {

}

/**
 * @brief   Open a file pointed by "path" and return its file descriptor on success, or error code otherwise
 */
utils::SyscallResult<GlobalFileDescriptor> VfsTree::open(const UnixPath& path) {
    auto fd = get_or_bring_entry_to_cache(path);

    if (!fd) {
        klog.format("VfsTree::open: can't open '%s'", path);
        return {fd.ec}; // propagate error code
    }

    // open increases refcount
    entry_cache[fd.value]->refcount++;
    return fd;
}

/**
 * @brief   Close the file and remove it from cache if no longer needed
 */
utils::SyscallResult<void> VfsTree::close(GlobalFileDescriptor fd) {
    if (!entry_cache.is_in_cache(fd))
        return {ErrorCode::EC_BADF};

    entry_cache[fd]->refcount--;

    // remove from cache if no longer needed. This needs testing
    if (entry_cache[fd]->refcount > 0 && entry_cache[fd]->attachment_count() > 0)
        return {ErrorCode::EC_PERM};


    entry_cache.deallocate(fd);
    return {ErrorCode::EC_OK};
}

/**
 * @brief   Remove entry from cache
 */
bool VfsTree::uncache(const UnixPath& path) {
    bool uncached {false};

    // try uncache directly stored entry
    if (auto fd = entry_cache.find(path)) {
        entry_cache.deallocate(fd.value);
        uncached = true;
    }

    // try detach parent-child
    if (auto fd = entry_cache.find(path.extract_directory())) {
        entry_cache[fd.value]->detach_entry(path.extract_file_name());
        uncached = true;
    }

    return uncached;
}

/**
 * @brief   Remove entry from persistent storage
 */
bool VfsTree::uncreate(const UnixPath& path) {
    // find the mountpoint responsible for managing the "path"
    auto mp = get_mountpoint_path(path);
    if (!mp) {
//        klog.format("VfsTree::remove: target located on unmodifiable filesystem: %\n", path);
//        return {ErrorCode::EC_ROFS};
        return false;
    }

    // make it remove the entry
    auto result = mp.mountpoint->delete_entry(mp.path);
    if (!result) {
//        klog.format("VfsTree::remove: target filesystem refused to remove entry: %\n", path);
//        return {result.ec};
        return false;
    }

    return true;
}

/**
 * @brief   Get the entry pointed by "path" from cache or bring it from virtual file system to cache
 *          and return its file descriptor or error code on error
 */
utils::SyscallResult<GlobalFileDescriptor> VfsTree::get_or_bring_entry_to_cache(const UnixPath& path) {
    // if file already in cache, just return the file descriptor
    if (auto fd = entry_cache.find(path))
        return {fd.value};

    // otherwise lookup the file, allocate it in cache and return the file descriptor
    VfsEntryPtr e = lookup_entry(path);
    if (!e) {
        klog.format("VfsTree::get_or_bring_entry_to_cache: entry '%s' not exists", path);
        return {ErrorCode::EC_NOENT};
    }

    auto fd = entry_cache.allocate(e, path);
    if (!fd) {
        klog.format("VfsTree::get_or_bring_entry_to_cache: open file limit reached");
        return {ErrorCode::EC_MFILE};
    }

    return {fd.value};
}

/**
 * @brief   Check for entry existence at "path"
 */
bool VfsTree::exists(const UnixPath& path) const {
    if (entry_cache.find(path))
        return true;

    if (lookup_entry(path))
        return true;

    return false;
}

/**
 * @brief   Lookup entry that is directly cached or directly cached attachment
 */
VfsCachedEntryPtr VfsTree::lookup_cached_entry(const UnixPath& path) const {
    if (auto fd = entry_cache.find(path))
        return entry_cache[fd.value];

    if (auto fd = entry_cache.find(path.extract_directory()))
        return entry_cache[fd.value]->get_attached_entry(path.extract_file_name());

    return {};
}

/**
 * @brief   Lookup entry pointed by "path" in the depths of virtual file system.
 *          Directly cached entries are not taken into account, but their children are,
 *          eg. for mountpoints that live in cache but their children are maintained by mountpoints themselves
 */
VfsEntryPtr VfsTree::lookup_entry(const UnixPath& path) const {
    List<string> path_segments;
    string subpath = path;
    VfsEntryPtr e;

    // ascend up the path eventually to the root if no parent is found in cache
    do {
        string segment = snap_path_tail(subpath);
        path_segments.push_front(segment);
        e = lookup_cached_entry(subpath);
    } while (!e);

    // now descent from the parent along the remembered segments
    for (const auto& path_segment : path_segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY) {
            klog.format("VfsTree::lookup_entry: path segment '%' is not a directory\n", e->get_name());
            return {};   // path segment is not a directory. this is error
        }

        e = get_entry_in_dir(path_segment, e);
        if (!e) {
            klog.format("VfsTree::lookup_entry: path segment '%' does not exist\n", path_segment);
            return {};   // path segment does not exist. this is error
        }
    }

    return e;
}

/**
 * @brief   Get mountpoint installed on the "path" and a new path relative to that mountpoint
 *          eg. for /mnt/USB/pictures/logo.jpg
 *              mountpoint would be USB and resulting path would be /pictures/logo.jpg
 * @param   path Absolute unix path
 * @return  Mountpoint and relative path, if there is a mountpoint installed on the "path", empty otherwise
 */
MountpointPath VfsTree::get_mountpoint_path(const cstd::string& path) {
    string subpath = path;
    string relative_path;
    VfsCachedEntryPtr e;

    // ascend up the path looking for the mountpoint in cache. Mountpoint always lives as attachment in cache
    do {
        e = lookup_cached_entry(subpath);
        if (e && e->is_mountpoint())
            break;
        string segment = snap_path_tail(subpath);
        relative_path = "/" + segment + relative_path;

    } while (subpath != "/");

    return {e, relative_path};
}

} /* namespace filesystem */
