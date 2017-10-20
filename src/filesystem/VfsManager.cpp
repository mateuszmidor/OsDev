/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "fat32/MassStorageMsDos.h"
#include "adapters/VfsFat32Entry.h"
#include "adapters/VfsFat32MountPoint.h"
#include "DriverManager.h"
#include "VfsManager.h"

using namespace kstd;
using namespace drivers;
namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install elementary filesystem entries under "/", including available ata volumes
 */
void VfsManager::install_root() {
    root = std::make_shared<VfsRamEntry>("/", true);

    VfsRamEntryPtr dev = std::make_shared<VfsRamEntry>("dev", true);
    VfsRamEntryPtr stdout = std::make_shared<VfsRamEntry>("stdout", false);
    dev->entries.push_back(stdout);
    root->entries.push_back(dev);

    VfsRamEntryPtr mnt = std::make_shared<VfsRamEntry>("mnt", true);
    root->entries.push_back(mnt);
    install_ata_devices(mnt);
}

/**
 * @brief   Get any entry that exists in virtual file system
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Pointer to entry if found, nullptr otherwise
 */
VfsEntryPtr VfsManager::get_entry(const UnixPath& unix_path) const {
    if (!root) {
        klog.format("VfsManager::get_entry: no root is installed\n");
        return nullptr;
    }

    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VfsManager::get_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return nullptr;
    }

    // start at root...
    VfsEntryPtr e = root;

    // ...and descend down the path to the very last entry
    auto segments = kstd::split_string<vector<string>>(unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e->is_directory()) {
            klog.format("VfsManager::get_entry: entry '%' is not a directory\n", e->get_name());
            return nullptr;   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsManager::get_entry: entry '%' does not exist\n", path_segment);
            return nullptr;   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return e;
}

/**
 * @brief   Create new file/directory for given unix path
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Entry pointer if created successfully, nullptr entry otherwise
 */
VfsEntryPtr VfsManager::create_entry(const UnixPath& unix_path, bool is_directory) const {
    string mountpoint_path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(mountpoint_path);
    if (mount_point)
        return mount_point->create_entry(mountpoint_path, is_directory);
    else {
        klog.format("VfsManager::create_entry: unmodifiable filesystem: %\n", unix_path);
        return nullptr;
    }
}

/**
 * @brief   Delete file or empty directory
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  True on successful deletion, False otherwise
 */
bool VfsManager::delete_entry(const UnixPath& unix_path) const {
    string mountpoint_path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(mountpoint_path);
    if (mount_point)
        return mount_point->delete_entry(mountpoint_path);
    else {
        klog.format("VfsManager::delete_entry: unmodifiable filesystem: %\n", unix_path);
        return false;
    }
}

/**
 * @brief   Move file/directory within virtual filesystem
 * @param   unix_path_from Absolute source entry path
 * @param   unix_path_to Absolute destination path/destination directory to move the entry into
 * @return  True on success, False otherwise
 */
bool VfsManager::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    string mountpoint_path_from = unix_path_from;
    VfsMountPointPtr mount_point_from = get_mountpoint_path(mountpoint_path_from);
    if (!mount_point_from) {
        klog.format("VfsManager::move_entry: unmodifiable src filesystem: %\n", unix_path_from);
        return false;
    }

    // check if moving mountpoint itself, cant do that
    if (mountpoint_path_from == "/") {
        klog.format("VfsManager::move_entry: can't move mountpoint just like that :) %\n", unix_path_from);
        return false;
    }

    string mountpoint_path_to = unix_path_to;
    VfsMountPointPtr mount_point_to = get_mountpoint_path(mountpoint_path_to);
    if (!mount_point_to) {
        klog.format("VfsManager::move_entry: unmodifiable dst filesystem: %\n", unix_path_to);
        return false;
    }

    // check if move within same mount point, this is more optimal scenario
    if (mount_point_from == mount_point_to) {
        klog.format("VfsManager::move_entry: moving within same mountpoint: % -> %\n", unix_path_from, unix_path_to);
        return mount_point_from->move_entry(mountpoint_path_from, mountpoint_path_to);
    }

    // nope, move between different mountpoints
    klog.format("VfsManager::move_entry: moving between 2 mountpoint: % -> %\n", unix_path_from, unix_path_to);
    if (copy_entry(unix_path_from, unix_path_to))
        return mount_point_from->delete_entry(mountpoint_path_from);
    else
        return false;
}

/**
 * @brief   Copy file within virtual filesystem. Copying entire directories not available; use make_entry(is_dir=true) + copy_entry
 * @param   unix_path_from Absolute source entry path
 * @param   unix_path_to Absolute destination path/destination directory to copy the entry into
 * @return  True on success, False otherwise
 */
bool VfsManager::copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    // source must exist
    VfsEntryPtr src = get_entry(unix_path_from);
    if (!src) {
        klog.format("VfsManager::copy_entry: source doesnt exist: %\n", unix_path_from);
        return false;
    }

    // source must be a file not a directory
    if (src->is_directory()) {
        klog.format("VfsManager::copy_entry: source is a directory. Use create_entry + copy_entry: %\n", unix_path_from);
        return false;
    }

    // destination must have its managing mountpoint
    string mountpoint_path_to = unix_path_to;
    VfsMountPointPtr mount_point_to = get_mountpoint_path(mountpoint_path_to);
    if (!mount_point_to) {
        klog.format("VfsManager::copy_entry: unmodifiable dst filesystem: %\n", unix_path_to);
        return false;
    }

    // check if unix_path_to describes destination directory or full destination path including filename
    UnixPath final_path_to;
    VfsEntryPtr dst = mount_point_to->get_entry(mountpoint_path_to);
    if (dst && dst->is_directory())
        final_path_to = format("%/%", unix_path_to, unix_path_from.extract_file_name());
    else
        final_path_to = unix_path_to;

    // create actual dest entry
    dst = create_entry(final_path_to, src->is_directory());
    if (!dst) {
        klog.format("VfsManager::copy_entry: can't create dst entry '%'\n", final_path_to);
        return false;
    }

    // dest entry created, just copy contents
    const u32 BUFF_SIZE = 1024;
    char buff[BUFF_SIZE]; // static to make sure recursive calls dont exhaust task stack
    u32 count;
    while ((count = src->read(buff, BUFF_SIZE)) > 0) {
        dst->write(buff, count);
    }

    return true;
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, empty entry otherwise
 */
VfsEntryPtr VfsManager::get_entry_for_name(VfsEntryPtr parent_dir, const string& name) const {
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
 * @brief   Install all ata devices/volumes under "parent"
 */
void VfsManager::install_ata_devices(VfsRamEntryPtr parent) {
    auto& driver_manager = DriverManager::instance();

    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus)
        return;

    if (ata_primary_bus->master_hdd.is_present()) {
        install_volumes(ata_primary_bus->master_hdd, parent);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        install_volumes(ata_primary_bus->slave_hdd, parent);
    }

    auto ata_secondary_bus = driver_manager.get_driver<AtaSecondaryBusDriver>();
    if (!ata_secondary_bus)
        return;

    if (ata_secondary_bus->master_hdd.is_present()) {
        install_volumes(ata_secondary_bus->master_hdd, parent);
    }

    if (ata_secondary_bus->slave_hdd.is_present()) {
        install_volumes(ata_secondary_bus->slave_hdd, parent);
    }
}

/**
 * @brief   Install all volumes available in "hdd" under "parent"
 */
void VfsManager::install_volumes(AtaDevice& hdd, VfsRamEntryPtr parent) {
    if (!MassStorageMsDos::verify(hdd))
        return;

    if (!parent)
        return;

    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        VfsEntryPtr drive = std::make_shared<VfsFat32MountPoint>(v);
        parent->entries.push_back(drive);
    }
}

/**
 * @brief   Get mountpoint installed on the "path" and cut "path" to start at the mountpoint
 *          eg. for /mnt/USB/pictures/logo.jpg
 *              mountpoint would be USB and resulting path would be /pictures/logo.jpg
 * @param   path Absolute unix path
 * @return  Mountpoint if found, nullptr otherwise
 * @note    Path without mountpoint is unmodifiable as there is no manager that can modify it
 */
VfsMountPointPtr VfsManager::get_mountpoint_path(kstd::string& path) const {
    if (!root) {
        klog.format("VfsManager::get_mountpoint_path: no root is installed\n");
        return nullptr;
    }

    VfsEntryPtr e = root;
    do  {
        string path_segment = snap_head(path, '/');
        if (path_segment.empty())
            continue;

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsManager::get_mountpoint_path: entry '%' does not exist\n", path_segment);
            return nullptr;
        }

        if (e->is_mount_point()) {
            path = "/" + path;  // return absolute path
            return std::static_pointer_cast<VfsMountPoint>(e);
        }
    } while (!path.empty());

    klog.format("VfsManager::get_mountpoint_path: no mountpoint installed on path: '%'\n", path);
    return nullptr;
}
} /* namespace filesystem */
