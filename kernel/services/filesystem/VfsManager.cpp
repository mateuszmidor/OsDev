/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "StringUtils.h"
#include "VfsManager.h"
#include "VfsRamFifoEntry.h"


namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install filesystem root and available ata volumes under the root
 */
void VfsManager::install() {
    persistent_storage.install();
}

/**
 * @brief   Get an entry that exists somewhere under the root
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Pointer to entry if found, nullptr otherwise
 */
VfsEntryPtr VfsManager::get_entry(const UnixPath& unix_path) {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return {};
    }

    if (auto e = cache_storage.get_entry(unix_path))
        return e;

    if (auto e = persistent_storage.get_entry(unix_path)) {
        cache_storage.add_entry(unix_path, e);
        return e;
    }

    return {};
}

void VfsManager::close_entry(VfsEntryPtr& entry) {
//    entry->close();
//    cache.remove_entry(entry); // but only remove if entry has no references
}

/**
 * @brief   Create new file/directory for given unix path
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Entry pointer if created successfully, nullptr entry otherwise
 */
VfsEntryPtr VfsManager::create_entry(const UnixPath& unix_path, bool is_directory) {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::create_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return {};
    }

    if (auto e = persistent_storage.create_entry(unix_path, is_directory)) {
        cache_storage.add_entry(unix_path, e);
        return e;
    }

    return {};
}

/**
 * @brief   Delete file or empty directory
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  True on successful deletion, False otherwise
 */
bool VfsManager::delete_entry(const UnixPath& unix_path) {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::delete_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return false;
    }

    bool cache_deleted = cache_storage.delete_entry(unix_path);
    bool persistent_deleted = persistent_storage.delete_entry(unix_path);
    return cache_deleted || persistent_deleted;
}

/**
 * @brief   Move file/directory within virtual filesystem
 * @param   unix_path_from Absolute source entry path
 * @param   unix_path_to Absolute destination path/destination directory to move the entry into
 * @return  True on success, False otherwise
 */
bool VfsManager::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    if (!unix_path_from.is_valid_absolute_path()) {
        klog.format("VfsManager::move_entry: path_from '%' is empty or it is not an absolute path\n", unix_path_from);
        return false;
    }

    if (!unix_path_to.is_valid_absolute_path()) {
        klog.format("VfsManager::move_entry: path_to '%' is empty or it is not an absolute path\n", unix_path_to);
        return false;
    }

    bool cache_moved = cache_storage.move_entry(unix_path_from, unix_path_to);
    bool persistent_moved = persistent_storage.move_entry(unix_path_from, unix_path_to);
    return cache_moved || persistent_moved;
}

/**
 * @brief   Copy file within virtual filesystem. Copying entire directories not available; use make_entry(is_dir=true) + copy_entry
 * @param   unix_path_from Absolute source entry path
 * @param   unix_path_to Absolute destination path/destination directory to copy the entry into
 * @return  True on success, False otherwise
 */
bool VfsManager::copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    if (!unix_path_from.is_valid_absolute_path()) {
        klog.format("VfsManager::copy_entry: path_from '%' is empty or it is not an absolute path\n", unix_path_from);
        return false;
    }

    if (!unix_path_to.is_valid_absolute_path()) {
        klog.format("VfsManager::copy_entry: path_to '%' is empty or it is not an absolute path\n", unix_path_to);
        return false;
    }

    bool cache_copied = cache_storage.copy_entry(unix_path_from, unix_path_to);
    bool persistent_copied = persistent_storage.copy_entry(unix_path_from, unix_path_to);
    return cache_copied || persistent_copied;
}

/**
 * @brief   Attach a ram-only fifo entry at "unix_path", if the path is valid
 * @param   unix_path Absolute path for the fifo to be created
 * @return  Newly created fifo pointer on success, empty pointer otherwise
 */
VfsEntryPtr VfsManager::create_fifo(const UnixPath& unix_path) {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::create_fifo: path '%' is empty or it is not an absolute path\n", unix_path);
        return {};
    }

    auto dir = unix_path.extract_directory();
    auto parent = get_entry(dir);
    if (!parent || !parent->is_directory()) {
        klog.format("VfsManager::create_fifo: invalid directory path: %\n", dir);
        return {};
    }

    auto fifo_name = unix_path.extract_file_name();

    auto on_entry = [&fifo_name](VfsEntryPtr e) -> bool {
        return (e->get_name() != fifo_name); // continue enumerating if name doesnt match
    };

    // such name already exists
    if (parent->enumerate_entries(on_entry) == VfsEnumerateResult::ENUMERATION_STOPPED) {
        klog.format("VfsManager::create_fifo: name % already exists in %\n", fifo_name, dir);
        return {};
    }

    auto fifo = std::make_shared<VfsRamFifoEntry>(fifo_name);
    parent->attach_entry(fifo);
    return fifo;
}

} /* namespace filesystem */
