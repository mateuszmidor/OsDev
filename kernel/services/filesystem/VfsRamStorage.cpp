/**
 *   @file: OpenEntryCache.cpp
 *
 *   @date: Jan 29, 2018
 * @author: Mateusz Midor
 */

#include "VfsRamStorage.h"

namespace filesystem {

VfsEntryPtr VfsRamStorage::get_entry(const UnixPath& path) {
    auto it = entries.find(path);
    if (it != entries.end())
        return it->second.lock();
    else
        return {};
}

void VfsRamStorage::add_entry(const UnixPath& path, VfsEntryPtr entry) {
    entries[path] = entry;
}

bool VfsRamStorage::delete_entry(const UnixPath& path) {
    bool anything_deleted {false};

    // remove cached persistent entry
    auto it = entries.find(path);
    if (it != entries.end()) {
         entries.erase(it);
         anything_deleted = true;
    }

    // remove cached non-persistent entry
    if (auto parent = get_entry(path.extract_directory())) {
        parent->detach_entry(path.extract_file_name());
        anything_deleted = true;
    }

    return anything_deleted;
}

bool VfsRamStorage::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    if (!copy_entry(unix_path_from, unix_path_to))
        return false;

    delete_entry(unix_path_from);
    return true;
}

bool VfsRamStorage::copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) {
    // check for dest path already exists
    if (get_entry(unix_path_to))
        return false;

    if (auto e = get_entry(unix_path_from)) {
        add_entry(unix_path_to, e);
        return true;
    }

    // there was nothing to copy, its ok
    return true;
}
} /* namespace filesystem */
