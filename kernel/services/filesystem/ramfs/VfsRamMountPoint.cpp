/**
 *   @file: VfsRamMountPoint.cpp
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#include "VfsRamMountPoint.h"
#include "VfsRamDummyFileEntry.h"
#include "StringUtils.h"
#include "Requests.h"

using namespace cstd;
using namespace middlespace;

namespace filesystem {

VfsRamMountPoint::VfsRamMountPoint(const cstd::string& name) : name(name) {
    root = cstd::make_shared<VfsRamDirectoryEntry>(name);
}

/**
 * @brief   Get Entry under given "path"
 * @note    This method covers both interfaces: Mountpoint::get(path) and Directory::get(name)
 */
utils::SyscallResult<VfsEntryPtr> VfsRamMountPoint::get_entry(const UnixPath& path) {
    if (!path.is_valid_path()) {
        requests->log("VfsRamMountPoint::get_entry: not a valid path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // start at root...
    VfsEntryPtr e {root};

    // ...and descend down the path to the very last entry
    auto segments = StringUtils::split_string(path, '/');
    for (const auto& path_segment : segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY) {
            requests->log("VfsRamMountPoint::get_entry: path segment '%' is not a directory\n", e->get_name());
            return {ErrorCode::EC_INVAL};   // path segment is not a directory. this is error
        }

        e = e->get_entry(path_segment).value;
        if (!e) {
            requests->log("VfsRamMountPoint::get_entry: path segment '%' does not exist\n", path_segment);
            return {ErrorCode::EC_NOENT};   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return {e};
}

/**
 * @brief   Enumerate entries under the root
 * @note    This method covers the Directory interface
 */
utils::SyscallResult<void> VfsRamMountPoint::enumerate_entries(const OnVfsEntryFound& on_entry) {
    return root->enumerate_entries(on_entry);
}

/**
 * @brief   Create an entry under given "path"
 */
utils::SyscallResult<VfsEntryPtr> VfsRamMountPoint::create_entry(const UnixPath& path, bool is_directory) {
    if (!path.is_valid_absolute_path()) {
        requests->log("VfsRamMountPoint::create_entry: path is empty or it is not an absolute path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    auto parent_path = path.extract_directory();
    auto parent = get_entry(parent_path).value;
    if (!parent || parent->get_type() != VfsEntryType::DIRECTORY) {
        requests->log("VfsRamMountPoint::create_entry: invalid parent directory path: %\n", parent_path);
        return {ErrorCode::EC_INVAL};
    }

    VfsEntryPtr entry;
    auto entry_name = path.extract_file_name();
    if (is_directory)
        entry = cstd::make_shared<VfsRamDirectoryEntry>(entry_name);
    else
        entry = cstd::make_shared<VfsRamDummyFileEntry>(entry_name);

    auto parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(parent);
    if (!parent_dir->attach_entry(entry))  {
        requests->log("VfsRamMountPoint::create_entry: file already exists: %\n", path);
        return {ErrorCode::EC_EXIST};
    }

    return {entry};
}

/**
 * @brief   Delete an entry under given "path"
 */
utils::SyscallResult<void> VfsRamMountPoint::delete_entry(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        requests->log("VfsRamMountPoint::delete_entry: path is empty or it is not an absolute path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    if (path.is_root_path()) {
        requests->log("VfsRamMountPoint::delete_entry: can't delete the root: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // get entry parent dir to delete from
    auto parent_path = path.extract_directory();
    auto parent = get_entry(parent_path).value;
    if (!parent || parent->get_type() != VfsEntryType::DIRECTORY) {
        requests->log("VfsRamMountPoint::delete_entry: invalid parent directory path: %\n", parent_path);
        return {ErrorCode::EC_INVAL};
    }

    // check if target entry exists
    auto entry_name = path.extract_file_name();
    auto parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(parent);
    auto entry = parent_dir->get_entry(entry_name).value;
    if (!entry) {
        requests->log("VfsRamMountPoint::delete_entry: invalid path: %\n", path);
        return {ErrorCode::EC_NOENT};
    }

    // check if target entry is nonempty directory
    if (is_non_empty_directory(entry)) {
        requests->log("VfsRamMountPoint::delete_entry: can't delete non-empty directory: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // delete entry from the parent
    if (parent_dir->detach_entry(entry_name))
        return {ErrorCode::EC_OK};
    else
        return {ErrorCode::EC_NOENT};
}

/**
 * @brief   Check if entry "e" is a directory that holds some contents
 */
bool VfsRamMountPoint::is_non_empty_directory(const VfsEntryPtr& e) const {
    return (e->get_type() == VfsEntryType::DIRECTORY && (std::static_pointer_cast<VfsRamDirectoryEntry>(e))->is_empty() == false);
}

/**
 * @brief   Move an entry from "path_from" to "path_to" within the filesystem
 * @param   path_from   Absolute path to source entry
 * @param   path_to     Absolute path to destination entry (not just its directory)
 */
utils::SyscallResult<void> VfsRamMountPoint::move_entry(const UnixPath& path_from, const UnixPath& path_to) {
    if (path_from.is_root_path()) {
        requests->log("VfsRamMountPoint::move_entry: can't move the root: %\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_from.is_valid_absolute_path()) {
        requests->log("VfsRamMountPoint::move_entry: path_from is empty or it is not an absolute path: %\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_to.is_valid_absolute_path()) {
        requests->log("VfsRamMountPoint::move_entry: path_to is empty or it is not an absolute path: %\n", path_to);
        return {ErrorCode::EC_INVAL};
    }

    auto src = get_entry(path_from);
    if (!src) {
        requests->log("VfsRamMountPoint::move_entry: src doesnt exist: %\n", path_from);
        return {ErrorCode::EC_NOENT};
    }

    string parent_path_to = path_to.extract_directory();
    auto dst_parent = get_entry(parent_path_to);
    if (!dst_parent) {
        requests->log("VfsRamMountPoint::move_entry: dst parent dir doesnt exist: %\n", parent_path_to);
        return {dst_parent.ec};
    }

    // check if only renaming entry within same folder
    auto entry = src.value;
    string dst_filename = path_to.extract_file_name();
    string parent_path_from = path_from.extract_directory();
    if (parent_path_from == parent_path_to) {
        entry->set_name(dst_filename);
        return {ErrorCode::EC_OK};
    }

    // moving to different directory
    auto src_parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(get_entry(parent_path_from).value);
    auto dst_parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(dst_parent.value);
    if (auto attach_result = dst_parent_dir->attach_entry(entry)) {
        string src_filename = path_from.extract_file_name();
        src_parent_dir->detach_entry(src_filename);// only detach if attach was successful so the entry doesnt disappear in case of error
        entry->set_name(dst_filename);
        return {ErrorCode::EC_OK};
    }
    else {
        requests->log("VfsRamMountPoint::move_entry: dst exists: %\n", path_to);
        return {ErrorCode::EC_EXIST};
    }
}
} /* namespace filesystem */
