/**
 *   @file: Fat32Data.cpp
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "Fat32Data.h"

using std::array;
using kstd::string;
using kstd::rtrim;
namespace filesystem {

Fat32Data::Fat32Data(drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Data::setup(u32 data_start_in_sectors, u8 sectors_per_cluster) {
    DATA_START_IN_SECTORS = data_start_in_sectors;
    SECTORS_PER_CLUSTER = sectors_per_cluster;
}

bool Fat32Data::read_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.read28(DATA_START_IN_SECTORS + SECTORS_PER_CLUSTER * (cluster - 2) + sector_offset, data, size);
}

bool Fat32Data::write_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.write28(DATA_START_IN_SECTORS + SECTORS_PER_CLUSTER * (cluster - 2) + sector_offset, data, size);
}

EnumerateResult Fat32Data::enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry) const {
    array<DirectoryEntryFat32, 16> entries;

    for (u8 sector_offset = 0; sector_offset < SECTORS_PER_CLUSTER; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (usually 512 bytes)
        read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
            const auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE)
                return EnumerateResult::ENUMERATION_FINISHED;    // no more entries for this dir

            if (e.name[0] == DIR_ENTRY_UNUSED)
                continue;       // unused entry, skip

            if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                continue;       // partition label

            if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                continue;   // extension for 8.3 filename

            if (!on_entry(make_simple_dentry(e, cluster, sector_offset, i)))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE; // continue reading the entries in next cluster
}

void Fat32Data::alloc_entry(const SimpleDentryFat32& parent_dir, const kstd::string& name, bool directory) const {

}

void Fat32Data::write_entry(const SimpleDentryFat32 &e) const {
    array<DirectoryEntryFat32, 16> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index] = make_directory_entry_fat32(e);
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void Fat32Data::release_entry(const SimpleDentryFat32& e) const {
    array<DirectoryEntryFat32, 16> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_UNUSED; // mark entry as deleted
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void Fat32Data::set_entry_data_cluster(const SimpleDentryFat32& e, u32 first_cluster) const {
    // update head
    array<DirectoryEntryFat32, 16> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].first_cluster_lo = first_cluster & 0xFFFF;
    entries[e.entry_index].first_cluster_hi = first_cluster >> 16;
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

bool Fat32Data::is_directory_cluster_empty(u32 cluster) const {
        auto on_entry = [](const SimpleDentryFat32& e) -> bool {
            if (e.name == "." || e.name == "..") // skip . and ..
                return true;

            return false; // file found, stop enumeration
        };
    return (enumerate_directory_cluster(cluster, on_entry) != EnumerateResult::ENUMERATION_STOPPED);
}

SimpleDentryFat32 Fat32Data::make_simple_dentry(const DirectoryEntryFat32& dentry, u32 entry_cluster, u16 entry_sector, u8 entry_index) const {
    string name = rtrim(dentry.name, sizeof(dentry.name));
    string ext = rtrim(dentry.ext, sizeof(dentry.ext));

    return SimpleDentryFat32(
                ext.empty() ? name : name + "." + ext,
                dentry.size,
                (dentry.attributes & DirectoryEntryFat32Attrib::DIRECTORY) == DirectoryEntryFat32Attrib::DIRECTORY,
                dentry.first_cluster_hi << 16 | dentry.first_cluster_lo,
                entry_cluster,
                entry_sector,
                entry_index
            );

}

DirectoryEntryFat32 Fat32Data::make_directory_entry_fat32(const SimpleDentryFat32& e) const {
    DirectoryEntryFat32 result;
    string name, ext;

    kstd::make_8_3_space_padded_filename(e.name, name, ext);
    memcpy(result.name, name.data(), 8);
    memcpy(result.ext, ext.data(), 3);
    result.a_time = 0;
    result.w_date = 0;
    result.w_time = 0;
    result.c_date = 0;
    result.c_time = 0;
    result.c_time_tenth = 0;
    result.attributes = e.is_directory ? DirectoryEntryFat32Attrib::DIRECTORY : 0;
    result.first_cluster_hi = e.data_cluster & 0xFF00;
    result.first_cluster_lo = e.data_cluster & 0x00FF;
    result.reserved = 0;
    result.size = e.size;

    return result;
}

} /* namespace filesystem */
