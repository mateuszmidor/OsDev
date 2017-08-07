/**
 *   @file: Fat32Entry.cpp
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Entry.h"
#include "Fat32Utils.h"

using namespace kstd;
namespace filesystem {

Fat32Entry::Fat32Entry() :
        Fat32Entry("", 0, false, 0, 0, 0, 0) {
}

Fat32Entry::Fat32Entry(const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 entry_cluster, u16 entry_sector, u8 entry_index_no) :
    name(name),
    size(size),
    is_directory(is_directory),
    data_cluster(data_cluster),
    entry_cluster(entry_cluster),
    entry_sector(entry_sector),
    entry_index(entry_index_no),
    position(0),
    position_data_cluster(data_cluster) {
}

Fat32Entry Fat32Entry::make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return Fat32Entry(
                ext.empty() ? name : name + "." + ext,
                dentry.size,
                (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
                dentry.first_cluster_hi << 16 | dentry.first_cluster_lo,
                entry_cluster,
                entry_sector,
                entry_index
            );

}

DirectoryEntryFat32 Fat32Entry::make_directory_entry_fat32(const Fat32Entry& e) {
    DirectoryEntryFat32 result;
    string name, ext;

    Fat32Utils::make_8_3_space_padded_filename(e.name, name, ext);
    memcpy(result.name, name.data(), 8);
    memcpy(result.ext, ext.data(), 3);
    result.a_time = 0;
    result.w_date = 0;
    result.w_time = 0;
    result.c_date = 0;
    result.c_time = 0;
    result.c_time_tenth = 0;
    result.attributes = e.is_directory ? DirectoryEntryFat32Attrib::DIRECTORY : 0;
    result.first_cluster_hi = e.data_cluster >> 16;
    result.first_cluster_lo = e.data_cluster & 0xFFFF;
    result.reserved = 0;
    result.size = e.size;

    return result;
}
} /* namespace filesystem */
