/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "MassStorageMsDos.h"
#include "DriverManager.h"
#include "VfsManager.h"
#include "VfsFat32Entry.h"

using namespace kstd;
using namespace drivers;
namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

void VfsManager::install_root() {
    root.name = "/";
    root.is_dir = true;

    VfsPseudoEntry* mnt = new VfsPseudoEntry;
    mnt->name = "mnt";
    mnt->is_dir = true;
    root.entries.push_back(mnt);

    install_ata_devices();
}

VfsEntry* VfsManager::get_entry(const UnixPath& unix_path) {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return nullptr;
    }

    // start at root...
    VfsEntry* e = &root;

    // ...and descend down the path to the very last entry
    auto normalized_unix_path = unix_path.normalize(); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e->is_directory()) {
            klog.format("VfsManager::get_entry: entry '%' is not a directory\n", e->get_name());
            return nullptr;   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(*e, path_segment);
        if (!e) {
            klog.format("VfsManager::get_entry: entry '%' does not exist\n", path_segment);
            return nullptr;   // path segment does not exist. this is error
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
VfsEntry* VfsManager::get_entry_for_name(VfsEntry& parent_dir, const string& name) {
    VfsEntry* result = nullptr;
    auto on_entry = [&](VfsEntry& e) -> bool {
        if (e.get_name() == name) {
            result = e.clone();
            return false;   // entry found. stop enumeration
        }
        else
            return true;    // continue searching for entry
    };

    parent_dir.enumerate_entries(on_entry);
    return result;
}


void VfsManager::install_ata_devices() {
    auto& driver_manager = DriverManager::instance();
    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus)
        return;

    if (ata_primary_bus->master_hdd.is_present()) {
        install_volumes(ata_primary_bus->master_hdd);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        install_volumes(ata_primary_bus->slave_hdd);
    }
}

void VfsManager::install_volumes(AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd))
        return;

    VfsPseudoEntry* mnt = (VfsPseudoEntry*)get_entry("/mnt");
    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        VfsFat32Entry* drive = new VfsFat32Entry(v.get_entry("/"));
        drive->set_custom_name(v.get_label());
        mnt->entries.push_back(drive);
    }
}
} /* namespace filesystem */
