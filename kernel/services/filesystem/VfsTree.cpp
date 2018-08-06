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
 * @brief   Get child entry in "parent_dir"
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
 * @brief   Check if given cached entry is unused and thus can be removed from cache
 * @return  EC_OK if unused, error code explaining how if used
 */
static utils::SyscallResult<void> check_cached_entry_unused(const VfsCachedEntryPtr& e) {
    if (e->open_count > 0) {
        return {ErrorCode::EC_ISOPEN};
    }

    if (e->attachment_count() > 0) {
        return {ErrorCode::EC_NOTEMPTY};
    }

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Copy contents of "src" to "dst", return error code on error
 */
static utils::SyscallResult<void> copy_entry_contents(VfsEntryPtr& src, VfsEntryPtr& dst) {
    // dest entry created, just copy contents
    const u32 BUFF_SIZE {1024};
    char buff[BUFF_SIZE];
    while (true) {
        auto read_result = src->read(buff, BUFF_SIZE);
        if (!read_result)
            return {read_result.ec};

        if (read_result.value == 0)
            break;

        auto write_result = dst->write(buff, read_result.value);
        if (!write_result)
            return {write_result.ec};

        if (write_result.value == 0)
            break;
    }
    return {ErrorCode::EC_OK};
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
 * @brief   Attach "entry" under given "parent_path" directory, return error code on error
 * @param   entry       Entry to be attached in virtual filesystem tree
 * @param   parent_path Absolute path to directory to attach the entry to
 */
utils::SyscallResult<void> VfsTree::attach(const VfsEntryPtr& entry, const UnixPath& parent_path) {
    if (!parent_path.is_valid_absolute_path()) {
        klog.format("VfsTree::attach: parent_path is empty or it is not an absolute path: %\n", parent_path);
        return {ErrorCode::EC_INVAL};
    }

    // get the parent from depths of virtual file system and cache it so we can attach to it
    auto parent_fd = get_or_bring_entry_to_cache(parent_path);
    if (!parent_fd) {
        klog.format("VfsTree::attach: can't attach to: %\n", parent_path);
        return {parent_fd.ec};
    }

    // check if parent is a directory
    if (entry_cache[parent_fd.value]->get_type() != VfsEntryType::DIRECTORY) {
        klog.format("VfsTree::attach: parent_path points to a non-directory: %\n", parent_path);
        uncache_if_unused(parent_fd.value);
        return {ErrorCode::EC_NOTDIR};
    }

    // attach entry to parent
    if (!entry_cache[parent_fd.value]->attach_entry(entry)) {
        klog.format("VfsTree::attach: entry '%' already exists at '%'\n", entry->get_name(), parent_path);
        return {ErrorCode::EC_EXIST};
    }

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Create file/directory pointed by "path" and return its file descriptor on success, or error code on error
 * @param   path    Absolute path to the entry that is to be created
 * @note    The actual entry creation is delegated to a mountpoint installed on the "path" thus mountpoint is a must
 */
utils::SyscallResult<GlobalFileDescriptor> VfsTree::create(const UnixPath& path, bool is_directory) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsTree::create: path is empty or it is not an absolute path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // find the mountpoint responsible for managing the "path"
    auto mp = get_mountpoint_path(path);
    if (!mp) {
        klog.format("VfsTree::create: target located on unmodifiable filesystem: %\n", path);
        return {ErrorCode::EC_ROFS};
    }

    // make the mountpoint create the entry
    auto create_result = mp.mountpoint->create_entry(mp.path, is_directory);
    if (!create_result) {
        klog.format("VfsTree::create: target filesystem refused to create entry: %\n", path);
        return {create_result.ec};
    }

    // cache the created entry so it can be accessed with filedescriptor
    auto fd = entry_cache.allocate(create_result.value, path);
    if (!fd) {
        klog.format("VfsTree::create: open file limit reached\n");
        return {ErrorCode::EC_MFILE};
    }

    // create also opens, and open increases refcount
    entry_cache[fd.value]->open_count++;
    return {fd.value};
}

/**
 * @brief   Delete entry pointed by "path" stored in filesystem
 * @param   path    Absolute path to the entry to be deleted
 */
utils::SyscallResult<void> VfsTree::remove(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsTree::remove: path is empty or it is not an absolute path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // try remove attached entry
    auto unattach_result = try_unattach(path);

    // check if entry is eligible for removing
    switch (unattach_result.ec) {
    case ErrorCode::EC_NOTEMPTY:
        klog.format("VfsTree::remove: can't remove; entry is not empty: %\n", path);
        return {ErrorCode::EC_NOTEMPTY};

    case ErrorCode::EC_ISOPEN:
        klog.format("VfsTree::remove: can't remove; entry is open: %\n", path);
        return {ErrorCode::EC_ISOPEN};

    case ErrorCode::EC_NOENT:  // no such attached entry, it can yet be lurking under some mountpoint so we continue...
        break;
    }

    // try remove persistent entry
    auto uncreate_result = try_uncreate(path);

    // any removal is a success
    if ((bool)unattach_result || (bool)uncreate_result)
        return {ErrorCode::EC_OK};

    return {uncreate_result.ec};
}

/**
 * @brief   Copy persistent/attached entry to persistent location
 * @param   path_from   Absolute source path
 * @param   path_to     Absolute destination path, either full name or directory to copy the entry to
 * @note    Can't copy entire directory
 */
utils::SyscallResult<void> VfsTree::copy(const UnixPath& path_from, const UnixPath& path_to) {
    if (!path_from.is_valid_absolute_path()) {
        klog.format("VfsTree::copy: path_from  is empty or it is not an absolute path: %\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_to.is_valid_absolute_path()) {
        klog.format("VfsTree::copy: path_to is empty or it is not an absolute path: %\n", path_to);
        return {ErrorCode::EC_INVAL};
    }

    // source must exist
    VfsEntryPtr src = lookup_entry(path_from);
    if (!src) {
        klog.format("VfsTree::copy: src doesn't exist: %\n", path_from);
        return {ErrorCode::EC_NOENT};
    }

    // source must be a file, not a directory
    if (src->get_type() == VfsEntryType::DIRECTORY) {
        klog.format("VfsTree::copy: src is a directory: %\n", path_from);
        return {ErrorCode::EC_ISDIR};
    }

    // destination must have its managing mountpoint
    auto mp_to = get_mountpoint_path(path_to);
    if (!mp_to) {
        klog.format("VfsTree::copy: dst located on unmodifiable filesystem: %\n", path_to);
        return {ErrorCode::EC_ROFS};
    }

    UnixPath relative_path_to;
    VfsEntryPtr dst = lookup_entry(path_to);
    if (dst && dst->get_type() == VfsEntryType::DIRECTORY)
        relative_path_to = StringUtils::format("%/%", mp_to.path, path_from.extract_file_name());
    else
        relative_path_to = mp_to.path;

    // create actual dest entry
    if (auto result = mp_to.mountpoint->create_entry(relative_path_to, false))
        dst = result.value;
    else {
        klog.format("VfsTree::copy: target filesystem refused to create dst entry %\n", relative_path_to);
        return {result.ec};
    }

    return copy_entry_contents(src, dst);
}

/**
 * @brief   Move entry from one location to another, both persistent and attached entries are handled
 * @param   path_from   Absolute source path
 * @param   path_to     Absolute destination path, either full name or directory to move the entry to
 * @note    Open entry can't be moved until it is closed.
 */
utils::SyscallResult<void> VfsTree::move(const UnixPath& path_from, const UnixPath& path_to) {
    if (path_from.is_root_path()) {
        klog.format("VfsTree::move: cannot move the root: %\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_from.is_valid_absolute_path()) {
        klog.format("VfsTree::move: path_from '%' is empty or it is not an absolute path\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_to.is_valid_absolute_path()) {
        klog.format("VfsTree::move: path_to '%' is empty or it is not an absolute path\n", path_to);
        return {ErrorCode::EC_INVAL};
    }

    auto move_attached_result = move_attached_entry(path_from, path_to);
    auto move_persistent_result = move_persistent_entry(path_from, path_to);

    if ((bool)move_attached_result || (bool)move_persistent_result)
        return {ErrorCode::EC_OK};

    // if same error code - return it
    if (move_attached_result.ec == move_persistent_result.ec)
        return {move_attached_result.ec};

    // if different error codes - return permission denied; what better to return?
    return {ErrorCode::EC_PERM};
}

/**
 * @brief   Move entry that lives as an attachment
 * @note    Can move attachments only; open files should stay where they are until no one has them open
 */
utils::SyscallResult<void> VfsTree::move_attached_entry(const UnixPath& path_from, const UnixPath& path_to) {
    VfsCachedEntryPtr src = lookup_attached_entry(path_from);
    if (!src) {
        klog.format("VfsTree::move_attached_entry: can't move; source doesn't exist: %\n", path_from);
        return {ErrorCode::EC_NOENT};
    }

    // check if entry in cache and eligible for removal
    if (src->open_count > 0) {
        klog.format("VfsTree::move_attached_entry: can't move; source is open: %\n", path_from);
        return {ErrorCode::EC_ISOPEN};
    }

    UnixPath final_path_to;
    VfsCachedEntryPtr dst = lookup_attached_entry(path_to);
    if (dst && dst->get_type() == VfsEntryType::DIRECTORY)
        final_path_to = StringUtils::format("%/%", path_to, path_from.extract_file_name());
    else
        final_path_to = path_to;

    // check if only renaming entry within same folder
    if (path_from.extract_directory() == final_path_to.extract_directory()) {
        src->set_name(final_path_to.extract_file_name());
        return {ErrorCode::EC_OK};
    }

    // moving to different directory
    if (auto attach_result = attach(src->get_raw_entry(), final_path_to.extract_directory())) {
        try_unattach(path_from); // only uncache if attach was successful so the entry doesnt disappear in case of error
        src->set_name(final_path_to.extract_file_name());
        return {ErrorCode::EC_OK};
    }
    else
        return attach_result;
}

/**
 * @brief   Move entry that lives in mountpoint
 */
utils::SyscallResult<void> VfsTree::move_persistent_entry(const UnixPath& path_from, const UnixPath& path_to) {
    auto mp_from = get_mountpoint_path(path_from);
    if (!mp_from) {
        klog.format("VfsTree::move_persistent_entry: src located on unmodifiable filesystem: %\n", path_from);
        return {ErrorCode::EC_ROFS};
    }

    // check if moving mountpoint itself, cant do that
    if (mp_from.path == "/") {
        klog.format("VfsTree::move_persistent_entry: can't move a mountpoint just like that :) %\n", path_from);
        return {ErrorCode::EC_PERM};
    }

    auto mp_to = get_mountpoint_path(path_to);
    if (!mp_to) {
        klog.format("VfsTree::move_persistent_entry: dst located on unmodifiable filesystem: %\n", path_to);
        return {ErrorCode::EC_ROFS};
    }

    UnixPath relative_path_to;
    VfsEntryPtr dst = lookup_entry(path_to);
    if (dst && dst->get_type() == VfsEntryType::DIRECTORY)
        relative_path_to = StringUtils::format("%/%", mp_to.path, path_from.extract_file_name());
    else
        relative_path_to = mp_to.path;

    // check if move within same mount point, this is more optimal scenario
    if (mp_from.mountpoint == mp_to.mountpoint) {
        klog.format("VfsTree::move_persistent_entry: moving within same mountpoint: % -> %\n", path_from, relative_path_to);
        return mp_from.mountpoint->move_entry(mp_from.path, relative_path_to);
    }

    // nope, move between different mountpoints
    klog.format("VfsTree::move_persistent_entry: moving between 2 mountpoints: % -> %\n", path_from, path_to);
    auto copy_result = copy(path_from, path_to);
    if (!copy_result)
        return copy_result;

    auto delete_result = mp_from.mountpoint->delete_entry(mp_from.path);
    if (!delete_result)
        return delete_result;

    return {ErrorCode::EC_OK};
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
    entry_cache[fd.value]->open_count++;
    return fd;
}

/**
 * @brief   Close the file and remove it from cache if no longer needed
 */
utils::SyscallResult<void> VfsTree::close(GlobalFileDescriptor fd) {
    if (!entry_cache.is_in_cache(fd))
        return {ErrorCode::EC_BADF};

    entry_cache[fd]->open_count--;

    // remove from cache if no longer needed. This needs testing
    uncache_if_unused(fd);

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Remove attached entry if eligible for removal, return error code otherwise
 * @result  EC_NOENT    if no entry to unattach
 *          EC_ISOPEN   if entry open by someone
 *          EC_NOTEMPTY if entry has some attachments of its own
 *          EC_OK       if successfully unattached
 */
utils::SyscallResult<void> VfsTree::try_unattach(const UnixPath& path) {
    // find parent
    auto parent_fd = entry_cache.find(path.extract_directory());
    if (!parent_fd)
        return {ErrorCode::EC_NOENT};

    // find attachment
    auto entry = entry_cache[parent_fd.value]->get_attached_entry(path.extract_file_name());
    if (!entry)
        return {ErrorCode::EC_NOENT};

    // check if entry should be detached
    auto unused = check_cached_entry_unused(entry);
    if (!unused)
        return {unused.ec};

    // detach the entry
    entry_cache[parent_fd.value]->detach_entry(path.extract_file_name());

    // try uncache the entry so it doesnt linger in cache while it should already be gone
    if (auto fd = entry_cache.find(path))
        uncache_if_unused(fd.value);

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Remove entry from filesystem managed by mountpoint installed on the "path"
 */
utils::SyscallResult<void> VfsTree::try_uncreate(const UnixPath& path) {
    // find the mountpoint responsible for managing the "path"
    auto mp = get_mountpoint_path(path);
    if (!mp)
        return {ErrorCode::EC_ROFS};

    // make the mountpoint remove the entry
    auto result = mp.mountpoint->delete_entry(mp.path);
    if (!result)
        return {result.ec};

    return {ErrorCode::EC_OK};
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
        klog.format("VfsTree::get_or_bring_entry_to_cache: entry doesn't exists: %", path);
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
 * @brief   Remove given entry from cache if noone uses it
 */
void VfsTree::uncache_if_unused(GlobalFileDescriptor fd) {
    if (check_cached_entry_unused(entry_cache[fd]))
        entry_cache.deallocate(fd);
}

/**
 * @brief   Check for entry existence at "path"
 */
bool VfsTree::exists(const UnixPath& path) const {
    if (lookup_entry(path))
        return true;

    return false;
}

/**
 * @brief   Lookup entry that is either root "/" or one of it's attachments (any deep)
 */
VfsCachedEntryPtr VfsTree::lookup_attached_entry(const UnixPath& path) const {
    if (auto fd = entry_cache.find(path))
        return entry_cache[fd.value];

    if (auto fd = entry_cache.find(path.extract_directory()))
        return entry_cache[fd.value]->get_attached_entry(path.extract_file_name());

    return {};
}

/**
 * @brief   Lookup entry pointed by "path" in the depths of virtual file system.
 *          All entries in Virtual File System tree are considered
 */
VfsEntryPtr VfsTree::lookup_entry(const UnixPath& path) const {
    List<string> path_segments;
    string subpath = path;
    VfsEntryPtr e;

    // ascend up the path eventually to the root if no parent is found in cache
    do {
        string segment = snap_path_tail(subpath);
        e = lookup_attached_entry(subpath);
        if (!segment.empty()) path_segments.push_front(segment);
    } while (!e);

    // now descent from the parent along the remembered segments
    for (const auto& path_segment : path_segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY)
            return {};   // path segment is not a directory. this is error

        e = get_entry_in_dir(path_segment, e);
        if (!e)
            return {};   // path segment does not exist. this is error
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
        e = lookup_attached_entry(subpath);
        if (e && e->is_mountpoint())
            break;
        string segment = snap_path_tail(subpath);
        relative_path = "/" + segment + relative_path;

    } while (subpath != "/");

    return {e, relative_path};
}

} /* namespace filesystem */
