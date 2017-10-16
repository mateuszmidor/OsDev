/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */
#include "VfsManager.h"

using namespace kstd;
namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

void VfsManager::install_root() {
    root = new VfsEntry;
    root->name = "/";
    root->is_dir = true;

    VfsEntry* mnt = new VfsEntry;
    mnt->name = "mnt";
    mnt->is_dir = true;
    root->entries.push_back(mnt);

    VfsEntry* drive_a = new VfsEntry;
    drive_a->name = "drive_a";
    drive_a->is_dir = true;
    mnt->entries.push_back(drive_a);

    VfsEntry* drive_b = new VfsEntry;
    drive_b->name = "drive_b";
    drive_b->is_dir = true;
    mnt->entries.push_back(drive_b);
}

VfsEntry VfsManager::get_entry(const UnixPath& unix_path) const {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return VfsEntry();
    }

    // start at root...
    VfsEntry e = *root;

    // ...and descend down the path to the very last entry
    auto normalized_unix_path = unix_path.normalize(); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e.is_dir) {
            klog.format("VfsManager::get_entry: entry '%' is not a directory\n", e.name);
            return VfsEntry();   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsManager::get_entry: entry '%' does not exist\n", path_segment);
            return VfsEntry();   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return e;
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, empty entry otherwise
 */
VfsEntry VfsManager::get_entry_for_name(VfsEntry& parent_dir, const string& name) const {
    VfsEntry result = VfsEntry();
    auto on_entry = [&](const VfsEntry& e) -> bool {
        if (e.name == name) {
            result = e;
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };

    parent_dir.enumerate_entries(on_entry);
    return result;
}
} /* namespace filesystem */
