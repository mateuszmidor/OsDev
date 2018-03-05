/**
 *   @file: VfsRamMountPoint.cpp
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#include "VfsRamMountPoint.h"
#include "StringUtils.h"

using namespace cstd;

namespace filesystem {

VfsRamMountPoint::VfsRamMountPoint(const cstd::string& name) : name(name), klog(logging::KernelLog::instance()) {
    root = std::make_shared<VfsRamDirectoryEntry>(name);
}

/**
 * @brief   Get Entry under given "path"
 * @note    This method covers both interfaces: Mountpoint::get(path) and Directory::get(name)
 */
utils::SyscallResult<VfsEntryPtr> VfsRamMountPoint::get_entry(const UnixPath& path) {
    if (!path.is_valid_path()) {
        klog.format("VfsRamMountPoint::get_entry: path '%' is not a valid path\n", path);
        return {middlespace::ErrorCode::EC_INVAL};
    }

    // start at root...
    VfsEntryPtr e {root};

    // ...and descend down the path to the very last entry
    auto segments = StringUtils::split_string(path, '/');
    for (const auto& path_segment : segments) {
        if (e->get_type() != VfsEntryType::DIRECTORY) {
            klog.format("VfsRamMountPoint::get_entry: path segment '%' is not a directory\n", e->get_name());
            return {middlespace::ErrorCode::EC_INVAL};   // path segment is not a directory. this is error
        }

        e = e->get_entry(path_segment).value;
        if (!e) {
            klog.format("VfsRamMountPoint::get_entry: path segment '%' does not exist\n", path_segment);
            return {middlespace::ErrorCode::EC_NOENT};   // path segment does not exist. this is error
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
 * @note    Only directories can be created as for now - "is_directory" must equal true
 */
utils::SyscallResult<VfsEntryPtr> VfsRamMountPoint::create_entry(const UnixPath& path, bool is_directory) {
    if (!is_directory) {
        klog.format("VfsRamMountPoint::create_entry: can only create in-ram directories and no files");
        return {middlespace::ErrorCode::EC_INVAL};
    }

    auto parent_path = path.extract_directory();
    auto parent = get_entry(parent_path).value;
    if (!parent || parent->get_type() != VfsEntryType::DIRECTORY) {
        klog.format("VfsRamMountPoint::create_entry: invalid parent directory path: %\n", parent_path);
        return {middlespace::ErrorCode::EC_INVAL};
    }
    auto parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(parent);

    auto entry_name = path.extract_file_name();
    auto entry = std::make_shared<VfsRamDirectoryEntry>(entry_name);
    if (parent_dir->attach_entry(entry))
        return {entry};
    else
        return {middlespace::ErrorCode::EC_EXIST};
}

/**
 * @brief   Delete an entry under given "path"
 */
utils::SyscallResult<void> VfsRamMountPoint::delete_entry(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsRamMountPoint::delete_entry: path '%' is empty or it is not an absolute path\n", path);
        return {middlespace::ErrorCode::EC_INVAL};
    }

    if (path.is_root_path()) {
        klog.format("VfsRamMountPoint::delete_entry: path '%' is root. Cant delete root\n", path);
        return {middlespace::ErrorCode::EC_INVAL};
    }

    // get entry parent dir to delete from
    auto parent_path = path.extract_directory();
    auto parent = get_entry(parent_path).value;
    if (!parent || parent->get_type() != VfsEntryType::DIRECTORY) {
        klog.format("VfsRamMountPoint::delete_entry: invalid parent directory path: %\n", parent_path);
        return {middlespace::ErrorCode::EC_INVAL};
    }
    auto parent_dir = std::static_pointer_cast<VfsRamDirectoryEntry>(parent);

    // check if target entry exists
    auto entry_name = path.extract_file_name();
    auto entry = parent_dir->get_entry(entry_name).value;
    if (!entry) {
        klog.format("VfsRamMountPoint::delete_entry: invalid path: %\n", path);
        return {middlespace::ErrorCode::EC_NOENT};
    }

    // check if target entry is nonempty directory
    if (is_non_empty_directory(entry)) {
        klog.format("VfsRamMountPoint::delete_entry: can't delete non-empty directory: %\n", path);
        return {middlespace::ErrorCode::EC_INVAL};
    }

    // delete entry from the parent
    if (parent_dir->detach_entry(entry_name))
        return {middlespace::ErrorCode::EC_OK};
    else
        return {middlespace::ErrorCode::EC_NOENT};
}

/**
 * @brief   Check if entry "e" is a directory that holds some contents
 */
bool VfsRamMountPoint::is_non_empty_directory(const VfsEntryPtr& e) const {
    return (e->get_type() == VfsEntryType::DIRECTORY && (std::static_pointer_cast<VfsRamDirectoryEntry>(e))->is_empty() == false);
}

/**
 * @brief   Move an entry from one path to another within the filesystem
 */
utils::SyscallResult<void> VfsRamMountPoint::move_entry(const UnixPath& from, const UnixPath& to) {
    klog.format("VfsRamMountPoint::move_entry: not implemented yet.");
    return {INVALID_OP};
}
} /* namespace filesystem */
