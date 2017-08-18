/**
 *   @file: VolumeFat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Utils.h"
#include "VolumeFat32.h"

using namespace kstd;
using namespace utils;
namespace filesystem {

/**
 * Constructor.
 * @brief   Create a volume based on physical drive device, and partition offset
 * @param   hdd Physical device this volume is located on
 * @param   bootable Is this OS bootable volume?
 * @param   partition_offset_in_sectors Where the volume data starts on the device
 * @param   partition_size_in_sectors How big the volume is
 */
VolumeFat32::VolumeFat32(drivers::AtaDevice& hdd, bool bootable, u32 partition_offset_in_sectors, u32 partition_size_in_sectors) :
        hdd(hdd),
        bootable(bootable),
        partition_offset_in_sectors(partition_offset_in_sectors),
        partition_size_in_sectors(partition_size_in_sectors),
        fat_table(hdd),
        fat_data(hdd),
        klog(KernelLog::instance()) {

    hdd.read28(partition_offset_in_sectors, &vbr, sizeof(vbr));

    u32 fat_start = partition_offset_in_sectors + vbr.reserved_sectors;
    fat_table.setup(fat_start, vbr.bytes_per_sector,  vbr.sectors_per_cluster, vbr.fat_table_size_in_sectors);

    u32 data_start = fat_start + vbr.fat_table_size_in_sectors * vbr.fat_table_copies;
    fat_data.setup(data_start, vbr.bytes_per_sector, vbr.sectors_per_cluster);
}

string VolumeFat32::get_label() const {
    return rtrim(vbr.volume_label, sizeof(vbr.volume_label));
}

string VolumeFat32::get_type() const {
    return rtrim(vbr.fat_type_label, sizeof(vbr.fat_type_label));
}

u32 VolumeFat32::get_size_in_bytes() const {
    return partition_size_in_sectors * vbr.bytes_per_sector;
}

u32 VolumeFat32::get_used_space_in_bytes() const {
    return fat_table.get_used_space_in_clusters() * vbr.sectors_per_cluster * vbr.bytes_per_sector;
}

u32 VolumeFat32::get_cluster_size_in_bytes() const {
    return vbr.sectors_per_cluster * vbr.bytes_per_sector;
}

/**
 * @brief   Get file/directory entry for given path
 * @param   unix_path Absolute path to file/directory Eg.
 *          "/home/docs/myphoto.jpg"
 *          "/home/music/"
 *          "/home/music"
 *          "/./" and "/../" and "//" in path are also supported as they are part of Fat32 format, eg
 *          "/home/music/..
 * @return  Valid entry if exists, empty entry otherwise
 */
Fat32Entry VolumeFat32::get_entry(const UnixPath& unix_path) const {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VolumeFat32::get_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return empty_entry();
    }

    // start at root...
    Fat32Entry e = get_root_dentry();

    // ...and descend down the path to the very last entry
    auto normalized_unix_path = unix_path.normalize(); // this takes care of '.' and '..'
    auto segments = kstd::split_string<vector<string>>(normalized_unix_path, '/');
    for (const auto& path_segment : segments) {
        if (!e.is_dir) {
            klog.format("VolumeFat32::get_entry: entry '%' is not a directory\n", e.name);
            return empty_entry();   // path segment is not a directory. this is error
        }

        e = get_entry_for_name(e, path_segment);
        if (!e) {
            klog.format("VolumeFat32::get_entry: entry '%' does not exist\n", path_segment);
            return empty_entry();   // path segment does not exist. this is error
        }
    }

    // managed to descend to the very last element of the path, means element found
    return e;
}

/**
 * @brief   Create new file/directory for given unix path
 * @return  Valid entry if created successfully, empty entry otherwise
 */
Fat32Entry VolumeFat32::create_entry(const UnixPath& unix_path, bool is_directory) const {
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VolumeFat32::create_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return empty_entry();
    }

    // check if entry name specified
    string name = unix_path.extract_file_name();
    if (name.empty()) {
        klog.format("VolumeFat32::create_entry: Please specify name for entry to create: %\n", unix_path);
        return empty_entry();
    }

    // check if parent_dir exists
    string directory = unix_path.extract_directory();
    Fat32Entry parent_dir = get_entry(directory);
    if (!parent_dir) {
        klog.format("VolumeFat32::create_entry: Parent not exists: %\n", directory);
        return empty_entry();
    }

    // try get valid 8.3 name that is not in use yet. We are using 8.3 names until support for long names is implemented :)
    string name_8_3;
    if (!get_free_name_8_3(parent_dir, name, name_8_3)) {
        klog.format("VolumeFat32::create_entry: Entry already exists: %(full: %)\n", name_8_3, unix_path);
        return empty_entry();
    }

    // allocate entry in parent_dir
    Fat32Entry out = empty_entry();
    out.name = name_8_3;
    out.is_dir = is_directory;
    klog.format("VolumeFat32::create_entry: name is '%'\n", out.name);
    if (!parent_dir.alloc_entry_in_directory(out))
        return empty_entry();

    // if directory - alloc the required "." and ".." entries
    if (is_directory)
        out.alloc_dot_dot_entries();

    return out;
}

/**
 * @brief   Get a valid, unique 8.3 filename
 * @param   parent Folder where the name must be unique
 * @param   full_name Original name
 * @param   name_8_3 Result
 * @return  True if successfully generated unique 8_3 filename, False otherwise
 */
bool VolumeFat32::get_free_name_8_3(Fat32Entry& parent, const string& full_name, string& name_8_3) const {
    // case 1. full_name fits in 8_3 limits
    if (Fat32Utils::fits_in_8_3(full_name)) {
        klog.format("VolumeFat32::get_free_name_8_3: name fits in 8_3\n");

        // check if entry with such name exists
        name_8_3 = Fat32Utils::make_8_3_filename(full_name);
        if (get_entry_for_name(parent, name_8_3)) {
            klog.format("VolumeFat32::get_free_name_8_3: but such file already exists\n");
            return false;
        }
        klog.format("VolumeFat32::get_free_name_8_3: and no such file exists yet\n");
        return true;
    }

    // case 2. full_name exceeds 8_3 limits. Need generate name~x.ext version
    klog.format("VolumeFat32::get_free_name_8_3: name doesnt fit in 8_3\n");
    for (u8 i = 1; i < 255; i++) {
        name_8_3 = Fat32Utils::make_8_3_filename(full_name, i);
        if (!get_entry_for_name(parent, name_8_3)) {
            klog.format("VolumeFat32::get_free_name_8_3: generated name %\n", name_8_3);
            return true;
        }
    }
    klog.format("VolumeFat32::get_free_name_8_3: couldnt generate free name\n");
    return false;
}

/**
 * @brief   Delete file or empty directory
 * @return  True on successful deletion, False if:
 *          -empty path specified
 *          -root dir specified
 *          -path does not exist
 *          -path points to a dir that is not empty
 */
bool VolumeFat32::delete_entry(const UnixPath& unix_path) const{
    if (!unix_path.is_valid_absolute_path()) {
        klog.format("VolumeFat32::delete_entry: path '%' is empty or it is not an absolute path\n", unix_path);
        return false;
    }

    if (unix_path.is_root_path()) {
        klog.format("VolumeFat32::delete_entry: path '%' root. Cant delete root\n", unix_path);
        return empty_entry();
    }

    // get entry parent dir to be updated
    Fat32Entry parent_dir = get_entry(unix_path.extract_directory());
    if (!parent_dir) {
        klog.format("VolumeFat32::delete_entry: path doesn't exist: %\n", unix_path);
        return false;
    }

    // get entry to be deleted
    Fat32Entry e = get_entry_for_name(parent_dir, unix_path.extract_file_name());
    if (!e) {
        klog.format("VolumeFat32::delete_entry: path doesn't exist: %\n", unix_path);
        return false;
    }

    // for directory entry - ensure it is empty
    if (e.is_dir && !e.is_directory_empty()) {
        klog.format("VolumeFat32::delete_entry: can't delete non-empty directory: %\n", unix_path);
        return false;
    }

    // release entry  data
    e.data.free();

    return parent_dir.dealloc_entry_in_directory(e, vbr.root_cluster);
}

/**
 * @brief   Move file/directory within volume
 * @param   unix_path_from Exact source entry path
 * @param   unix_path_to Exact destination path/destination directory to move the entry into
 * @return  True on success, False otherwise
 */
bool VolumeFat32::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    if (!unix_path_from.is_valid_absolute_path()) {
        klog.format("VolumeFat32::move_entry: path from '%' is empty or it is not an absolute path\n", unix_path_from);
        return false;
    }

    if (!unix_path_to.is_valid_absolute_path()) {
        klog.format("VolumeFat32::move_entry: path to '%' is empty or it is not an absolute path\n", unix_path_to);
        return false;
    }

    // get source entry
    Fat32Entry src = get_entry(unix_path_from);
    if (!src) {
        klog.format("VolumeFat32::move_entry: src entry doesn't exists '%'\n", unix_path_from);
        return false;
    }

    // if destination directory specified instead of full path - keep source name
    string path_to;
    Fat32Entry dest = get_entry(unix_path_to);
    if (dest && dest.is_dir)
        path_to = format("%/%", unix_path_to, unix_path_from.extract_file_name());
    else
        path_to = unix_path_to;

    // create destination entry
    Fat32Entry dst = create_entry(path_to, src.is_dir);
    if (!dst) {
        klog.format("VolumeFat32::move_entry: can't create dst entry '%'\n", path_to);
        return false;
    }

    // move data cluster chain from src to dst entry, clear src data
    std::swap(src.data, dst.data);
    if (!src.update_entry_info_in_parent_dir())
        return false;

    if (!dst.update_entry_info_in_parent_dir())
        return false;

    // remove src entry
    if (!delete_entry(unix_path_from)) {
        klog.format("VolumeFat32::move_entry: can't remove src entry '%'\n", unix_path_from);
        return false;
    }

    return true;
}

/**
 * @brief   Return root directory entry; this is the entry point to entire volume dir tree
 */
Fat32Entry VolumeFat32::get_root_dentry() const {
    return Fat32Entry(fat_table, fat_data, "/", 0, true, vbr.root_cluster, Fat32Table::CLUSTER_END_OF_CHAIN, 0);
}

/**
 * @brief   Get entry in "parent_dir"
 * @param   name Entry name. Case sensitive
 * @return  Valid entry if exists, empty entry otherwise
 */
Fat32Entry VolumeFat32::get_entry_for_name(Fat32Entry& parent_dir, const string& name) const {
    Fat32Entry result = empty_entry();
    auto on_entry = [&](const Fat32Entry& e) -> bool {
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

Fat32Entry VolumeFat32::empty_entry() const {
    return Fat32Entry(fat_table, fat_data);
}


} /* namespace filesystem */
