/**
 *   @file: Fat32Entry.h
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32ENTRY_H_
#define SRC_FILESYSTEM_FAT32ENTRY_H_

#include <functional>
#include "kstd.h"
#include "KernelLog.h"
#include "Fat32Structs.h"
#include "Fat32Table.h"
#include "Fat32Data.h"
#include "Fat32ClusterChain.h"

namespace filesystem {

// directory enumeration result
enum class EnumerateResult { ENUMERATION_FINISHED, ENUMERATION_STOPPED, ENUMERATION_CONTINUE };

// action to take on entry enumeration. Return true to continue directory contents enumeration
class Fat32Entry;
using OnEntryFound = std::function<bool(Fat32Entry& e)>;


/**
 * @name    Fat32Entry
 * @brief   User friendly replacement for DirectoryEntryFat32. Represents file or directory
 */
class Fat32Entry {
public:
    static DirectoryEntryFat32 make_directory_entry_fat32(const Fat32Entry& e);

    // [common interface]
    Fat32Entry(const Fat32Entry& other) = default;
    Fat32Entry& operator=(const Fat32Entry& other);
    operator bool() const;
    bool operator!() const;
    bool is_directory() const;
    u32 get_size() const;
    const kstd::string& get_name() const;

    // [file interface]
    u32 read(void* data, u32 count);
    u32 write(const void* data, u32 count);
    void seek(u32 new_position);
    void truncate(u32 new_size);

    // [directory interface]
    EnumerateResult enumerate_entries(const OnEntryFound& on_entry);

private:
    friend class VolumeFat32;   // only VolumeFat32 can instantiate Fat32Entry

    // default constructor is private so Fat32Entry instance can only be obtained from VolumeFat32
    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data);
    Fat32Entry(const Fat32Table& fat_table, const Fat32Data& fat_data, const kstd::string& name, u32 size, bool is_directory, u32 data_cluster, u32 parent_data_cluster, u32 parent_index);

    void update_entry_info_in_parent_dir();
    EnumerateResult enumerate_directory_cluster(u32 cluster, const OnEntryFound& on_entry, u8 start_sector = 0, u8 start_index = 0) const;
    Fat32Entry make_simple_dentry(const DirectoryEntryFat32& dentry, Fat32ClusterChain parent_data, u32 parent_index) const;
    bool alloc_entry_in_directory(Fat32Entry& out);
    bool alloc_entry_in_directory_at_index(u32 entry_index, Fat32Entry& out);
    void mark_entry_as_nomore(Fat32Entry& e) const;
    void mark_next_entry_as_nomore(const Fat32Entry& e) const;
    bool is_initialized() const;

    static constexpr u8 FAT32ENTRIES_PER_SECTOR = 16; // BYTES_PER_SECTOR / sizeof(DirectoryEntryFat32);
    utils::KernelLog&   klog;
    const Fat32Table&   fat_table;
    const Fat32Data&    fat_data;

    // entry metadata
    kstd::string        name;
    Fat32ClusterChain   data;
    bool                is_dir;

    // entry localization in parent dir
    Fat32ClusterChain   parent_data;
    u32                 parent_index;   // DirectoryEntryFat32 index in parent_data
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32ENTRY_H_ */
