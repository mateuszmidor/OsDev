/**
 *   @file: VfsPersistentStorage.cpp
 *
 *   @date: Feb 2, 2018
 * @author: Mateusz Midor
 */

#include "VfsPersistentStorage.h"
#include "fat32/MassStorageMsDos.h"
#include "adapters/VfsFat32MountPoint.h"
#include "DriverManager.h"
#include "StringUtils.h"

using namespace cstd;
using namespace drivers;

namespace filesystem {

/**
 * @brief   Get any entry that exists in virtual file system
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Pointer to entry if found, nullptr otherwise
 */
VfsEntryPtr VfsPersistentStorage::get_entry(const UnixPath& unix_path) const {
    if (!root) {
        klog.format("VfsPersistentStorage::get_entry: no root is installed\n");
        return {};
    }

    // start at root...
    VfsEntryPtr e = root;

    // ...and descend down the path to the very last entry
    auto segments = StringUtils::split_string(unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e->is_directory()) {
            klog.format("VfsPersistentStorage::get_entry: path segment '%' is not a directory\n", e->get_name());
            return {};   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsPersistentStorage::get_entry: path segment '%' does not exist\n", path_segment);
            return {};   // path segment does not exist. this is error
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
VfsEntryPtr VfsPersistentStorage::create_entry(const UnixPath& unix_path, bool is_directory) const{
    string mountpoint_path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(mountpoint_path);
    if (mount_point)
        return mount_point->create_entry(mountpoint_path, is_directory);

    klog.format("VfsPersistentStorage::create_entry: target located on unmodifiable filesystem: %\n", unix_path);
    return {};
}

/**
 * @brief   Delete file or empty directory
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  True on successful deletion, False otherwise
 */
bool VfsPersistentStorage::delete_entry(const UnixPath& unix_path) const {
    string mountpoint_path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(mountpoint_path);
    if (mount_point)
        return mount_point->delete_entry(mountpoint_path);

    klog.format("VfsPersistentStorage::delete_entry: target located on unmodifiable filesystem: %\n", unix_path);
    return {};
}

/**
 * @brief   Move file/directory within virtual filesystem
 * @param   unix_path_from Absolute source entry path
 * @param   unix_path_to Absolute destination path/destination directory to move the entry into
 * @return  True on success, False otherwise
 */
bool VfsPersistentStorage::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    string mountpoint_path_from = unix_path_from;
    VfsMountPointPtr mount_point_from = get_mountpoint_path(mountpoint_path_from);
    if (!mount_point_from) {
        klog.format("VfsPersistentStorage::move_entry: src located on unmodifiable filesystem: %\n", unix_path_from);
        return false;
    }

    // check if moving mountpoint itself, cant do that
    if (mountpoint_path_from == "/") {
        klog.format("VfsPersistentStorage::move_entry: can't move mountpoint just like that :) %\n", unix_path_from);
        return false;
    }

    string mountpoint_path_to = unix_path_to;
    VfsMountPointPtr mount_point_to = get_mountpoint_path(mountpoint_path_to);
    if (!mount_point_to) {
        klog.format("VfsPersistentStorage::move_entry: dst located on unmodifiable filesystem: %\n", unix_path_to);
        return false;
    }

    // check if move within same mount point, this is more optimal scenario
    if (mount_point_from == mount_point_to) {
        klog.format("VfsPersistentStorage::move_entry: moving within same mountpoint: % -> %\n", unix_path_from, unix_path_to);
        return mount_point_from->move_entry(mountpoint_path_from, mountpoint_path_to);
    }

    // nope, move between different mountpoints
    klog.format("VfsPersistentStorage::move_entry: moving between 2 mountpoint: % -> %\n", unix_path_from, unix_path_to);
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
bool VfsPersistentStorage::copy_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    // source must exist
    VfsEntryPtr src = get_entry(unix_path_from);
    if (!src) {
        klog.format("VfsPersistentStorage::copy_entry: src doesnt exist: %\n", unix_path_from);
        return false;
    }

    // source must be a file not a directory
    if (src->is_directory()) {
        klog.format("VfsPersistentStorage::copy_entry: src is a directory. Use create_entry + copy_entry: %\n", unix_path_from);
        return false;
    }

    // destination must have its managing mountpoint
    string mountpoint_path_to = unix_path_to;
    VfsMountPointPtr mount_point_to = get_mountpoint_path(mountpoint_path_to);
    if (!mount_point_to) {
        klog.format("VfsPersistentStorage::copy_entry: dst located on unmodifiable filesystem: %\n", unix_path_to);
        return false;
    }

    // check if unix_path_to describes destination directory or full destination path including filename
    UnixPath final_path_to;
    VfsEntryPtr dst = mount_point_to->get_entry(mountpoint_path_to);
    if (dst && dst->is_directory())
        final_path_to = StringUtils::format("%/%", unix_path_to, unix_path_from.extract_file_name());
    else
        final_path_to = unix_path_to;

    // create actual dest entry
    dst = create_entry(final_path_to, src->is_directory());
    if (!dst) {
        klog.format("VfsPersistentStorage::copy_entry: can't create dst entry '%'\n", final_path_to);
        return false;
    }

    // dest entry created, just copy contents
    const u32 BUFF_SIZE = 1024;
    char buff[BUFF_SIZE]; // static to make sure recursive calls dont exhaust task stack
    u32 count;
    while ((count = src->read(buff, BUFF_SIZE)) > 0)
        dst->write(buff, count);

    return true;
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, empty entry otherwise
 */
VfsEntryPtr VfsPersistentStorage::get_entry_for_name(VfsEntryPtr parent_dir, const string& name) const {
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
 * @brief   Get mountpoint installed on the "path" and cut "path" to start at the mountpoint
 *          eg. for /mnt/USB/pictures/logo.jpg
 *              mountpoint would be USB and resulting path would be /pictures/logo.jpg
 * @param   path Absolute unix path
 * @return  Mountpoint if found, nullptr otherwise
 * @note    Path without mountpoint is unmodifiable as there is no manager that can modify it
 */
VfsMountPointPtr VfsPersistentStorage::get_mountpoint_path(string& path) const {
    if (!root) {
        klog.format("VfsPersistentStorage::get_mountpoint_path: no root is installed\n");
        return {};
    }

    VfsEntryPtr e = root;
    do  {
        string path_segment = StringUtils::snap_head(path, '/');
        if (path_segment.empty())
            continue;

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VfsPersistentStorage::get_mountpoint_path: path segment '%' does not exist\n", path_segment);
            return {};
        }

        if (e->is_mount_point()) {
            path = "/" + path;  // return absolute path
            return std::static_pointer_cast<VfsMountPoint>(e);
        }
    } while (!path.empty());

    klog.format("VfsPersistentStorage::get_mountpoint_path: no mountpoint installed on path: '%'\n", path);
    return {};
}

/**
 * @brief   Install all ata devices and volumes under "/"
 */
void VfsPersistentStorage::install() {
    root = std::make_shared<VfsRamDirectoryEntry>("/");
    auto& driver_manager = DriverManager::instance();

    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus)
        return;

    if (ata_primary_bus->master_hdd.is_present()) {
        install_volumes(ata_primary_bus->master_hdd, root);
    }

    if (ata_primary_bus->slave_hdd.is_present()) {
        install_volumes(ata_primary_bus->slave_hdd, root);
    }

    auto ata_secondary_bus = driver_manager.get_driver<AtaSecondaryBusDriver>();
    if (!ata_secondary_bus)
        return;

    if (ata_secondary_bus->master_hdd.is_present()) {
        install_volumes(ata_secondary_bus->master_hdd, root);
    }

    if (ata_secondary_bus->slave_hdd.is_present()) {
        install_volumes(ata_secondary_bus->slave_hdd, root);
    }
}

/**
 * @brief   Install all volumes available in "hdd" under "parent"
 */
void VfsPersistentStorage::install_volumes(AtaDevice& hdd, VfsRamDirectoryEntryPtr parent) {
    if (!parent)
        return;

    if (!MassStorageMsDos::verify(hdd))
        return;

    MassStorageMsDos ms(hdd);
    for (const auto& v : ms.get_volumes()) {
        VfsEntryPtr drive = std::make_shared<VfsFat32MountPoint>(v);
        parent->attach_entry(drive);
    }
}

} /* namespace filesystem */
