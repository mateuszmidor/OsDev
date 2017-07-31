/**
 *   @file: Fat32Data.cpp
 *
 *   @date: Jul 11, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "Fat32Data.h"
#include "Fat32Utils.h"

using std::array;
using kstd::string;
using kstd::rtrim;
namespace filesystem {

Fat32Data::Fat32Data(drivers::AtaDevice& hdd) :
    hdd(hdd) {
}

void Fat32Data::setup(u32 data_start_in_sectors, u16 bytes_per_sector, u8 sectors_per_cluster) {
    this->data_start_in_sectors = data_start_in_sectors;
    this->bytes_per_sector = bytes_per_sector;
    this->sectors_per_cluster = sectors_per_cluster;
}

bool Fat32Data::read_data_sector(u32 cluster, u8 sector_offset, void* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.read28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
}

/**
 * @brief Read "size" bytes of data at [cluster][sector_in_cluster][byte_in_sector]
 */
bool Fat32Data::read_data_sector_from_byte(u32 cluster, u8 sector_in_cluster, u16 byte_in_sector, void* data, u32 size) const {
    // read across sectors is not supported
    if (byte_in_sector + size > bytes_per_sector)
        return false;

    // read starts at sector beginning, optimal read
    if (byte_in_sector == 0)
        return read_data_sector(cluster, sector_in_cluster, data, size);


    // read starts somewhere in the middle of sector, need read entire sector and copy from buffer to data
    u8 buff[bytes_per_sector];
    if (!read_data_sector(cluster, sector_in_cluster, buff, bytes_per_sector))
        return false;

    memcpy(data, buff + byte_in_sector, size);
    return true;
}

bool Fat32Data::write_data_sector(u32 cluster, u8 sector_offset, void const* data, u32 size) const {
    // (cluster - 2) because data clusters are indexed from 2
    return hdd.write28(data_start_in_sectors + sectors_per_cluster * (cluster - 2) + sector_offset, data, size);
}

void Fat32Data::clear_data_cluster(u32 cluster) const {
    u8 zeroes[bytes_per_sector];
    memset(zeroes, 0, sizeof(zeroes));
    for (u8 sector_offset = 0; sector_offset < sectors_per_cluster; sector_offset++)
        write_data_sector(cluster, sector_offset, zeroes, sizeof(zeroes));
}

EnumerateResult Fat32Data::enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector, u8 start_index) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;

    for (u8 sector_offset = start_sector; sector_offset < sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (usually 512 bytes)
        read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = start_index; i < entries.size(); i++) { // iterate directory entries
            auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE)
                return EnumerateResult::ENUMERATION_FINISHED;    // no more entries for this dir

            if (e.name[0] == DIR_ENTRY_UNUSED)
                continue;       // unused entry, skip

            if ((e.attributes & DirectoryEntryFat32Attrib::VOLUMEID) == DirectoryEntryFat32Attrib::VOLUMEID)
                continue;       // partition label

            if ((e.attributes & DirectoryEntryFat32Attrib::LONGNAME) == DirectoryEntryFat32Attrib::LONGNAME)
                continue;   // extension for 8.3 filename

            SimpleDentryFat32 se = make_simple_dentry(e, cluster, sector_offset, i);
            if (!on_entry(se))
                return EnumerateResult::ENUMERATION_STOPPED;
        }
    }
    return EnumerateResult::ENUMERATION_CONTINUE; // continue reading the entries in next cluster
}

/**
 * @brief   Scan directory cluster looking for either UNUSED or NO_MORE entry
 * @param   out Free entry, if found, is stored here
 * @return  DIR_ENTRY_UNUSED, DIR_ENTRY_NO_MORE or 0xFF if not found
 */
u8 Fat32Data::get_free_entry_in_dir_cluster(u32 cluster, SimpleDentryFat32& out) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;

    for (u8 sector_offset = 0; sector_offset < sectors_per_cluster; sector_offset++) { // iterate sectors in cluster

        // read 1 sector of data (usually 512 bytes)
        read_data_sector(cluster, sector_offset, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());

        for (u8 i = 0; i < entries.size(); i++) { // iterate directory entries
            const auto& e = entries[i];
            if (e.name[0] == DIR_ENTRY_NO_MORE || e.name[0] == DIR_ENTRY_UNUSED) {
                out.entry_cluster = cluster;
                out.entry_sector = sector_offset;
                out.entry_index = i;
                return e.name[0];
            }
        }
    }
    return DIR_ENTRY_NOT_FOUND; // no free entry found in this cluster
}

void Fat32Data::write_entry(const SimpleDentryFat32& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index] = make_directory_entry_fat32(e);
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void Fat32Data::mark_entry_as_nomore(const SimpleDentryFat32& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_NO_MORE; // mark entry as last one
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void Fat32Data::mark_next_entry_as_nomore(const SimpleDentryFat32& e) const {
    // if e is the very last entry in the cluster - CLUSTER_END_OF_DIRECTORY will mark the end of directory entries
    if ((e.entry_sector == sectors_per_cluster - 1) && (e.entry_index == FAT32ENTRIES_PER_SECTOR - 1))
        return;

    SimpleDentryFat32 no_more;
    no_more.entry_cluster = e.entry_cluster;
    no_more.entry_sector = (e.entry_index < 15) ? e.entry_sector : e.entry_sector + 1;
    no_more.entry_index = (e.entry_index < 15) ? e.entry_index + 1 : 0;

    mark_entry_as_nomore(no_more);
}

void Fat32Data::mark_entry_as_unused(const SimpleDentryFat32& e) const {
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
    read_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
    entries[e.entry_index].name[0] = DIR_ENTRY_UNUSED; // mark entry as deleted
    write_data_sector(e.entry_cluster, e.entry_sector, entries.data(), sizeof(DirectoryEntryFat32) * entries.size());
}

void Fat32Data::set_entry_data_cluster(const SimpleDentryFat32& e, u32 first_cluster) const {
    // update head
    array<DirectoryEntryFat32, FAT32ENTRIES_PER_SECTOR> entries;
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
