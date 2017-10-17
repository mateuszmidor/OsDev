/**
 *   @file: VfsFat32MountPoint.cpp
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#include "VfsFat32Entry.h"
#include "VfsFat32MountPoint.h"

namespace filesystem {

VfsFat32MountPoint::VfsFat32MountPoint(const VolumeFat32& volume) : volume(volume), root(volume.get_entry("/")), name(volume.get_label()) {
}

VfsFat32MountPoint::~VfsFat32MountPoint() {
}

bool VfsFat32MountPoint::is_directory() const {
    return root.is_directory();
}

u32 VfsFat32MountPoint::get_size() const {
    return root.get_size();
}

const kstd::string& VfsFat32MountPoint::get_name() const {
    return name;
}

u32 VfsFat32MountPoint::read(void* data, u32 count) {
    return root.read(data, count);
}

u32 VfsFat32MountPoint::write(const void* data, u32 count) {
    return root.write(data, count);
}

bool VfsFat32MountPoint::seek(u32 new_position) {
    return root.seek(new_position);
}

bool VfsFat32MountPoint::truncate(u32 new_size) {
    return root.truncate(new_size);
}

/**
 * @brief   Enumerate directory contents
 * @param   on_entry Callback called for every valid element in the directory
 * @return  ENUMERATION_FINISHED if all entries have been enumerated,
 *          ENUMERATION_STOPPED if enumeration stopped by on_entry() returning false
 */
VfsEnumerateResult VfsFat32MountPoint::enumerate_entries(const OnVfsEntryFound& on_entry) {
    auto on_fat_entry = [&](const Fat32Entry& e) -> bool {
        VfsEntryPtr entry = std::make_shared<VfsFat32Entry>(e);
        return on_entry(entry);
    };

    return (VfsEnumerateResult) root.enumerate_entries(on_fat_entry);
}

VfsEntryPtr VfsFat32MountPoint::get_entry(const UnixPath& unix_path) {
    return std::make_shared<VfsFat32Entry>(volume.get_entry(unix_path));
}

VfsEntryPtr VfsFat32MountPoint::create_entry(const UnixPath& unix_path, bool is_directory) const {
    return std::make_shared<VfsFat32Entry>(volume.create_entry(unix_path, is_directory));
}

bool VfsFat32MountPoint::delete_entry(const UnixPath& unix_path) const {
    return volume.delete_entry(unix_path);
}

bool VfsFat32MountPoint::move_entry(const UnixPath& unix_path_from, const UnixPath& unix_path_to) const {
    return volume.move_entry(unix_path_from, unix_path_to);
}

} /* namespace filesystem */
