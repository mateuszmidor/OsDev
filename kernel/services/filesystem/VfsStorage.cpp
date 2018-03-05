/**
 *   @file: VfsStorage.cpp
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#include "VfsStorage.h"
#include "StringUtils.h"

using namespace cstd;

namespace filesystem {

/**
 * @brief   Check if entry under absolute "path" exists
 */
bool VfsStorage::entry_exists(const UnixPath& path) const {
    if (!root)
        return false;

    // start at root...
    VfsEntryPtr e = root;

    // ...and descend down the path to the very last entry
    const auto segments = StringUtils::split_string(path, '/');
    for (const auto& path_segment : segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY)
            return false;

        e = get_entry_for_name(e, path_segment);
        if (!e)
            return false;
    }
    return true;
}

/**
 * @brief   Get an entry that exists in virtual file system under absolute "path"
 */
VfsEntryPtr VfsStorage::get_entry(const UnixPath& path) const {
    if (!root) {
        klog.format("VfsStorage::get_entry: no root is installed\n");
        return {};
    }

    // start at root...
    VfsEntryPtr e = root;

    // ...and descend down the path to the very last entry
    const auto segments = StringUtils::split_string(path, '/');
    for (const auto& path_segment : segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY) {
            klog.format("VfsStorage::get_entry: path segment '%' is not a directory\n", e->get_name());
            return {};   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsStorage::get_entry: path segment '%' does not exist\n", path_segment);
            return {};   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return e;
}

/**
 * @brief   Create new file/directory under absolute "path"
 */
VfsEntryPtr VfsStorage::create_entry(const UnixPath& path, bool is_directory) const {
    auto mp = get_mountpoint_path(path);
    if (mp)
        return mp.mountpoint->create_entry(mp.path, is_directory).value;

    klog.format("VfsStorage::create_entry: target located on unmodifiable filesystem: %\n", path);
    return {};
}

/**
 * @brief   Delete file or an empty directory under absolute "path"
 */
utils::SyscallResult<void> VfsStorage::delete_entry(const UnixPath& path) const {
    if (auto mp = get_mountpoint_path(path))
        return mp.mountpoint->delete_entry(mp.path);

    klog.format("VfsStorage::delete_entry: target located on unmodifiable filesystem: %\n", path);
    return {middlespace::ErrorCode::EC_PERM};
}

/**
 * @brief   Move file/directory within virtual file system
 * @param   path_from Absolute source entry path
 * @param   path_to Absolute destination path/destination directory to move the entry into
 */
bool VfsStorage::move_entry(const UnixPath& path_from, const UnixPath& path_to) const {
    auto mp_from = get_mountpoint_path(path_from);
    if (!mp_from) {
        klog.format("VfsStorage::move_entry: src located on unmodifiable filesystem: %\n", path_from);
        return false;
    }

    // check if moving mountpoint itself, cant do that
    if (mp_from.path == "/") {
        klog.format("VfsStorage::move_entry: can't move a mountpoint just like that :) %\n", path_from);
        return false;
    }

    auto mp_to = get_mountpoint_path(path_to);
    if (!mp_to) {
        klog.format("VfsStorage::move_entry: dst located on unmodifiable filesystem: %\n", path_to);
        return false;
    }

    // check if move within same mount point, this is more optimal scenario
    if (mp_from.mountpoint == mp_to.mountpoint) {
        klog.format("VfsStorage::move_entry: moving within same mountpoint: % -> %\n", path_from, path_to);
        return (bool)mp_from.mountpoint->move_entry(mp_from.path, mp_to.path);
    }

    // nope, move between different mountpoints
    klog.format("VfsStorage::move_entry: moving between 2 mountpoints: % -> %\n", path_from, path_to);
    return copy_entry(path_from, path_to) && mp_from.mountpoint->delete_entry(mp_from.path);
}

/**
 * @brief   Copy file within virtual file system. Copying entire directories not available; use make_entry(is_dir=true) + copy_entry
 * @param   path_from Absolute source entry path
 * @param   path_to Absolute destination path/destination directory to copy the entry into
 */
bool VfsStorage::copy_entry(const UnixPath& path_from, const UnixPath& path_to) const {
    // source must exist
    VfsEntryPtr src = get_entry(path_from);
    if (!src) {
        klog.format("VfsStorage::copy_entry: src doesnt exist: %\n", path_from);
        return false;
    }

    // source must be a file, not a directory
    if (src->get_type() == VfsEntryType::DIRECTORY) {
        klog.format("VfsStorage::copy_entry: src is a directory: %\n", path_from);
        return false;
    }

    // destination must have its managing mountpoint
    auto mp_to = get_mountpoint_path(path_to);
    if (!mp_to) {
        klog.format("VfsStorage::copy_entry: dst located on unmodifiable filesystem: %\n", path_to);
        return false;
    }

    // check if path_to describes destination directory or full destination path including filename
    UnixPath final_path_to;
    VfsEntryPtr dst = mp_to.mountpoint->get_entry(mp_to.path).value;
    if (dst && dst->get_type() == VfsEntryType::DIRECTORY)
        final_path_to = StringUtils::format("%/%", path_to, path_from.extract_file_name());
    else
        final_path_to = path_to;

    // create actual dest entry
    dst = create_entry(final_path_to, false);
    if (!dst) {
        klog.format("VfsStorage::copy_entry: can't create dst entry '%'\n", final_path_to);
        return false;
    }

    // dest entry created, just copy contents
    const u32 BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    u32 count;
    while ((count = src->read(buff, BUFF_SIZE).value) > 0)
        dst->write(buff, count);

    return true;
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, nullptr otherwise
 */
VfsEntryPtr VfsStorage::get_entry_for_name(VfsEntryPtr parent_dir, const string& name) const {
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
 * @brief   Get mountpoint installed on the "path" and a new path relative to that mountpoint
 *          eg. for /mnt/USB/pictures/logo.jpg
 *              mountpoint would be USB and resulting path would be /pictures/logo.jpg
 * @param   path Absolute unix path
 * @return  Mountpoint and relative path, if there is a mountpoint installed on the "path", empty otherwise
 */
details::MountpointPath VfsStorage::get_mountpoint_path(const string& path) const {
    if (!root) {
        klog.format("VfsStorage::get_mountpoint_path: no root is installed\n");
        return {};
    }

    VfsEntryPtr deepest_mountpoint;
    string deepest_mountpoint_relative_path;

    VfsEntryPtr e = root;
    string remaining_path = path;
    do  {
        string path_segment = StringUtils::snap_head(remaining_path, '/');
        if (path_segment.empty())
            continue;

        e = get_entry_for_name(e, path_segment);
        if (!e)
            break;

        if (e->is_mountpoint()) {
            deepest_mountpoint = e;
            deepest_mountpoint_relative_path = remaining_path;
        }
    } while (!remaining_path.empty());

    if (!deepest_mountpoint) {
        klog.format("VfsStorage::get_mountpoint_path: no mountpoint installed on path: '%'\n", path);
        return {};
    }

    return {deepest_mountpoint, "/" + deepest_mountpoint_relative_path};
}

/**
 * @brief   Install all ata devices and volumes under "/"
 */
void VfsStorage::install() {
    root = std::make_shared<VfsRamDirectoryEntry>("/");
}

} /* namespace filesystem */
