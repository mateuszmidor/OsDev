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
 * @brief   Attach "entry" under given "parent_path"
 */
utils::SyscallResult<void> VfsTree::attach(const VfsEntryPtr& entry, const UnixPath& parent_path) {
    // cache the parent so we can attach to it
    auto fd = get_or_bring_entry_to_cache(parent_path);

    if (!fd) {
        klog.format("VfsTree::attach: can't attach to '%s'", parent_path);
        return {ErrorCode::EC_INVAL};
    }

    if (!cached_entries[fd.value]->attach_entry(entry)) {
        klog.format("VfsTree::attach: entry '%s' already exists at '%s'", entry->get_name(), parent_path);
        return {ErrorCode::EC_EXIST};
    }

    return {ErrorCode::EC_OK};
}

/**
 * @brief   Open a file at "path" and return its file descriptor on success, or error code otherwise
 */
utils::SyscallResult<GlobalFileDescriptor> VfsTree::open(const UnixPath& path) {
    auto fd = get_or_bring_entry_to_cache(path);

    if (!fd) {
        klog.format("VfsTree::open: can't open '%s'", path);
        return {fd.ec}; // propagate error code
    }

    // open increases refcount
    cached_entries[fd.value]->refcount++;
    return fd;
}

/**
 * @brief   Close the file and remove it from cache if no longer needed
 */
void VfsTree::close(GlobalFileDescriptor fd) {
    if (fd >= cached_entries.size())
        return;

    if (!cached_entries[fd])
        return;

    cached_entries[fd]->refcount--;

    // remove from cache if no longer needed. This needs testing
    if (cached_entries[fd]->refcount == 0 && cached_entries[fd]->attachment_count() == 0) {
        path_to_filedescriptor.erase(path_to_filedescriptor.find_by_val(fd));
        cached_entries[fd].reset();
    }
}

utils::SyscallResult<GlobalFileDescriptor> VfsTree::get_or_bring_entry_to_cache(const UnixPath& path) {
    // if file already in cache, just increment refcount and return the file descriptor
    if (auto fd = get_fd_for_path(path))
        return {fd.value};

    // otherwise lookup the file, allocate it in a table, increment refcount, set path-to-fd mapping and return the file descriptor
    VfsEntryPtr e = lookup_entry(path);
    if (!e) {
        klog.format("VfsTree::get_or_bring_entry_to_cache: entry '%s' not exists", path);
        return {ErrorCode::EC_NOENT};
    }

    auto fd = allocate_entry_in_open_table(e);
    if (!fd) {
        klog.format("VfsTree::get_or_bring_entry_to_cache: too many open files");
        return {ErrorCode::EC_MFILE};
    }

    set_fd_for_path(fd.value, path);

    return {fd.value};
}

bool VfsTree::exists(const UnixPath& path) const {
    if (get_fd_for_path(path))
        return true;

    if (lookup_entry(path))
        return true;

    return false;
}

/**
 * @return   Global file descriptor if file pointed by "path" is already open, empty otherwise
 */
Optional<GlobalFileDescriptor> VfsTree::get_fd_for_path(const UnixPath& path) const {
    const auto found = path_to_filedescriptor.find(path);
    if (found == path_to_filedescriptor.cend())
        return {};

    return {found->second};
}

void VfsTree::set_fd_for_path(GlobalFileDescriptor fd, const UnixPath& path) {
    path_to_filedescriptor[path] = fd;
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, nullptr otherwise
 */
static VfsEntryPtr get_entry_for_name(VfsEntryPtr parent_dir, const string& name) {
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

VfsEntryPtr VfsTree::lookup_entry(const UnixPath& path) const {
    List<string> segments;

    // first find the mountpoint that resides in cache
    string subpath = path;
    Optional<GlobalFileDescriptor> fd;
    while (!subpath.empty()) {
        string segment = StringUtils::snap_tail(subpath, '/');
        segments.push_front(segment);
        fd = get_fd_for_path(subpath);
        if (fd)
            break;
    }

    // mountpoint not found, try from root
    if (!fd)
        fd =  get_fd_for_path("/");

    // then now descent from mountpoint along the remembered segments
    VfsEntryPtr e = cached_entries[fd.value];

    for (const auto& path_segment : segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY) {
            klog.format("VfsTree::lookup_entry: path segment '%' is not a directory\n", e->get_name());
            return {};   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsTree::lookup_entry: path segment '%' does not exist\n", path_segment);
            return {};   // path segment does not exist. this is error
        }
    }

    return e;
}


Optional<GlobalFileDescriptor> VfsTree::allocate_entry_in_open_table(const VfsEntryPtr& e) {
    if (auto fd = find_free_fd_in_open_table()) {
        cached_entries[fd.value] = std::make_shared<VfsCachedEntry>(e);
        return {fd.value};
    }

    return {};
}

Optional<GlobalFileDescriptor> VfsTree::find_free_fd_in_open_table() const {
    for (u32 i = 0; i < cached_entries.size(); i++)
        if (!cached_entries[i])
            return {i};

    // free file descriptor not found
    klog.format("VfsTree::find_empty_entry_in_open_table: limit of globally open files %d reached", cached_entries.size());
    return {};
}

void VfsTree::install() {
    auto root_dir = std::make_shared<VfsRamDirectoryEntry>("/"); // empty dir
    auto cached_root_dir = std::make_shared<VfsCachedEntry>(root_dir);
    cached_entries.resize(128);

    cached_entries[0] = cached_root_dir;
    set_fd_for_path(0, "/");
}
} /* namespace filesystem */
