/**
 *   @file: Fat32Entry.cpp
 *
 *   @date: Aug 7, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Entry.h"

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

} /* namespace filesystem */
