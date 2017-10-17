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
#include "VfsFat32MountPoint.h"

using namespace kstd;
using namespace drivers;
namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install elementary filesystem entries under "/", including available ata devices/volumes
 */
void VfsManager::install_root() {
    std::shared_ptr<VfsPseudoEntry> pseudo_root = std::make_shared<VfsPseudoEntry>("/", true);

    VfsEntryPtr mnt = std::make_shared<VfsPseudoEntry>("mnt", true);
    pseudo_root->entries.push_back(mnt);
    root = pseudo_root;

    install_ata_devices();
}

/**
 * @brief   Get any entry currently available in virtual file system
 * @param   unix_path Absolute entry path starting at root "/"
 * @return  Pointer to entry if found, nullptr otherwise
 */
VfsEntryPtr VfsManager::get_entry(const UnixPath& unix_path) {
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
    auto normalized_unix_path = unix_path.normalize(); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
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
 * @return  Entry pointer if created successfully, nullptr entry otherwise
 */
VfsEntryPtr VfsManager::create_entry(const UnixPath& unix_path, bool is_directory) const {
    string path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(path);
    if (mount_point)
        return mount_point->create_entry(path, is_directory);
    else {
        klog.format("VfsManager::create_entry: unmodifiable filesystem: %\n", unix_path);
        return nullptr;
    }
}

/**
 * @brief   Delete file or empty directory
 * @return  True on successful deletion, False otherwise
 */
bool VfsManager::delete_entry(const UnixPath& unix_path) const {
    string path = unix_path;
    VfsMountPointPtr mount_point = get_mountpoint_path(path);
    if (mount_point)
        return mount_point->delete_entry(path);
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
    string path_from = unix_path_from;
    VfsMountPointPtr mount_point_from = get_mountpoint_path(path_from);
    if (!mount_point_from) {
        klog.format("VfsManager::move_entry: unmodifiable filesystem: %\n", unix_path_from);
        return false;
    }

    string path_to = unix_path_to;
    VfsMountPointPtr mount_point_to = get_mountpoint_path(path_to);
    if (!mount_point_to) {
        klog.format("VfsManager::move_entry: unmodifiable filesystem: %\n", unix_path_to);
        return false;
    }

    // check if move within same mount point
    if (mount_point_from == mount_point_to)
        return mount_point_from->move_entry(path_from, path_to);


    // nope, move betweed different mountpoints
    // copy_entry(unix_path_from, unix_path_to);
    // mount_point_from->delete_entry(unix_path_from);

    return false;
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
 * @brief   Install all devices under primary and secondary ata bus in "/mnt"
 */
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

    std::shared_ptr<VfsPseudoEntry> mnt = std::static_pointer_cast<VfsPseudoEntry>(get_entry("/mnt"));
    if (!mnt)
        return;

    MassStorageMsDos ms(hdd);
    for (auto& v : ms.get_volumes()) {
        VfsEntryPtr drive = std::make_shared<VfsFat32MountPoint>(v);
        mnt->entries.push_back(drive);
    }
}

/**
 * @brief   Get mountpoint installed on the "path", cut "path" to start at the mountpoint
 *          eg. for /mnt/USB/pictures/logo.jpg
 *              mountpoint would be USB and path would be /pictures/logo.jpg
 * @param   path Absolute path
 * @return  Mountpoint if found, nullptr otherwise
 * @note    Path without mountpoint is unmodifiable as there is no manager that can modify it
 */
VfsMountPointPtr VfsManager::get_mountpoint_path(kstd::string& path) const {
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
