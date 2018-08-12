/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "StringUtils.h"
#include "VfsManager.h"

using namespace utils;
using namespace middlespace;

namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install filesystem root and available ata volumes under the root
 */
void VfsManager::install_root() {
    storage.install();
    tree.install();
}

/**
 * @brief   Mount a filesystem under the root "/"
 */
utils::SyscallResult<void> VfsManager::mount(const VfsEntryPtr& mount_point) {
    if (!mount_point->is_mountpoint()) {
        klog.format("VfsManager::mount: provided entry is not a mountpoint\n");
        return {ErrorCode::EC_INVAL};
    }

    tree.attach(mount_point, "/");

    auto root_dir = storage.get_root();
    if (!root_dir)
        return {ErrorCode::EC_NOENT};   // root doesn't exist

    if (root_dir->attach_entry(mount_point))
        return {ErrorCode::EC_OK};

    auto attach_result = tree.attach(mount_point, "/");
    if (!attach_result)
        return attach_result.ec;

    klog.format("VfsManager::mount: path '/%' already exists\n", mount_point->get_name());
    return {ErrorCode::EC_EXIST};   // entry already exists under the root
}

/**
 * @brief   Check if absolute "path" exists
 */
utils::SyscallResult<void> VfsManager::entry_exists(const UnixPath& path) const {
    if (directory_cache.entry_exists(path))
        return {ErrorCode::EC_OK};

    if (storage.entry_exists(path))
        return {ErrorCode::EC_OK};

    return {ErrorCode::EC_NOENT};
}

/**
 * @brief   Get an entry that exists under "path"
 * @param   path Absolute entry path starting at root "/"
 */
SyscallResult<VfsEntryPtr> VfsManager::get_entry(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_entry: path '%' is empty or it is not an absolute path\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // check for entry in directory cache; cached directory can hold extra-attachment entries
    if (auto e = directory_cache.get_entry(path))
        return {e};

    // check for entry in actual storage
    if (auto e = storage.get_entry(path))
        return {e};

    // path doesnt exist
    return {ErrorCode::EC_NOENT};
}

/**
 * @brief   Get directory entry from cache or bring it to cache if not there yet
 * @return  Error if "path" doesnt exists or points to a non-directory
 * @note    Watch out for caching files; they contain state (current read/write position) that should not be shared between different tasks
 */
utils::SyscallResult<VfsCachedEntryPtr> VfsManager::get_or_cache_directory(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_cached_entry: path '%' is empty or it is not an absolute path\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // check for entry in cache
    if (auto cached = directory_cache.get_entry(path))
        if (cached->get_type() == VfsEntryType::DIRECTORY)
            return {cached};
        else
            return {ErrorCode::EC_INVAL};

    // check for entry in actual storage
    if (auto e = storage.get_entry(path))
        if (e->get_type() == VfsEntryType::DIRECTORY)
            return directory_cache.add_entry(path, e);
        else
            return {ErrorCode::EC_INVAL};

    return {ErrorCode::EC_NOENT};
}

//void VfsManager::close_entry(VfsEntryPtr& entry) {
//    entry->close();
//    cache.remove_entry(entry); // but only remove if entry has no references
//}

/**
 * @brief   Create new file/directory under given absolute "path"
 */
utils::SyscallResult<VfsEntryPtr> VfsManager::create_entry(const UnixPath& path, bool is_directory) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsManager::create_entry: path '%' is empty or it is not an absolute path\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // check such entry already exists
    if (entry_exists(path)) {
        klog.format("VfsManager::create_entry: entry % already exists\n", path);
        return {ErrorCode::EC_EXIST};
    }

    if (auto e = storage.create_entry(path, is_directory))
        return {e};

    return {ErrorCode::EC_PERM};
}

/**
 * @brief   Delete file or empty directory
 * @param   path Absolute entry path starting at root "/"
 */
utils::SyscallResult<void> VfsManager::delete_entry(const UnixPath& path) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsManager::delete_entry: path '%' is empty or it is not an absolute path\n", path);
        return {ErrorCode::EC_INVAL};
    }

    // entry may be in cache, but must be in storage or it is an error to delete
    bool cache_deleted = directory_cache.delete_entry(path);
    auto storage_delete_result = storage.delete_entry(path);
    return (cache_deleted || storage_delete_result) ? ErrorCode::EC_OK : storage_delete_result.ec;
}

/**
 * @brief   Move file/directory within virtual filesystem
 * @param   path_from Absolute source entry path
 * @param   path_to Absolute destination path/destination directory to move the entry into
 */
utils::SyscallResult<void> VfsManager::move_entry(const UnixPath& path_from, const UnixPath& path_to) {
    if (!path_from.is_valid_absolute_path()) {
        klog.format("VfsManager::move_entry: path_from '%' is empty or it is not an absolute path\n", path_from);
        return {ErrorCode::EC_INVAL};
    }

    if (!path_to.is_valid_absolute_path()) {
        klog.format("VfsManager::move_entry: path_to '%' is empty or it is not an absolute path\n", path_to);
        return {ErrorCode::EC_INVAL};
    }

    bool cache_moved = directory_cache.move_entry(path_from, path_to);
    bool storage_moved = storage.move_entry(path_from, path_to);
    return (cache_moved || storage_moved) ? ErrorCode::EC_OK : ErrorCode::EC_NOENT;
}

/**
 * @brief   Copy file within virtual filesystem. Copying entire directories not available; use make_entry(is_dir=true) + copy_entry
 * @param   path_from Absolute source entry path
 * @param   path_to Absolute destination path/destination directory to copy the entry into
 * @return  True on success, False otherwise
 */
//bool VfsManager::copy_entry(const UnixPath& path_from, const UnixPath& path_to) {
//    if (!path_from.is_valid_absolute_path()) {
//        klog.format("VfsManager::copy_entry: path_from '%' is empty or it is not an absolute path\n", path_from);
//        return false;
//    }
//
//    if (!path_to.is_valid_absolute_path()) {
//        klog.format("VfsManager::copy_entry: path_to '%' is empty or it is not an absolute path\n", path_to);
//        return false;
//    }
//
//    bool cache_copied = cache.copy_entry(path_from, path_to);
//    bool persistent_copied = storage.copy_entry(path_from, path_to);
//    return cache_copied || persistent_copied;
//}

/**
 * @brief   Attach "entry" at given "path", where "path" must point to a DIRECTORY
 */
utils::SyscallResult<void> VfsManager::attach_special(const UnixPath& path, const VfsEntryPtr& entry) {
    if (!path.is_valid_absolute_path()) {
        klog.format("VfsManager::attach_special: path '%' is empty or it is not an absolute path\n", path);
        return {ErrorCode::EC_INVAL};
    }

    auto parent = get_or_cache_directory(path).value;
    if (!parent) {
        klog.format("VfsManager::attach_special: invalid parent directory path: %\n", path);
        return {ErrorCode::EC_INVAL};
    }

    return (parent->attach_entry(entry)) ? ErrorCode::EC_OK : ErrorCode::EC_EXIST;
}

utils::SyscallResult<void> VfsManager::attach(const VfsEntryPtr& entry, const UnixPath& path) {
    return tree.attach(entry, path);
}

utils::SyscallResult<GlobalFileDescriptor> VfsManager::create(const UnixPath& path, bool is_directory) {
    return tree.create(path, is_directory);
}

utils::SyscallResult<void> VfsManager::remove(const UnixPath& path) {
    return tree.remove(path);
}

utils::SyscallResult<void> VfsManager::copy(const UnixPath& path_from, const UnixPath& path_to) {
    return tree.copy(path_from, path_to);
}

utils::SyscallResult<void> VfsManager::move(const UnixPath& path_from, const UnixPath& path_to) {
    return tree.move(path_from, path_to);
}

utils::SyscallResult<GlobalFileDescriptor> VfsManager::open(const UnixPath& path) {
    return tree.open(path);
}

utils::SyscallResult<void> VfsManager::close(GlobalFileDescriptor fd) {
    return tree.close(fd);
}

bool VfsManager::exists(const UnixPath& path) const {
    return tree.exists(path);
}

utils::SyscallResult<u64> VfsManager::get_size(GlobalFileDescriptor fd) const {
    auto e = tree.get_entry(fd);
    return e.entry->get_size();
}

utils::SyscallResult<u64> VfsManager::read(GlobalFileDescriptor fd, void* data, u32 count) {
    auto e = tree.get_entry(fd);
    return e.entry->read(data, count);
}
} /* namespace filesystem */
