/**
 *   @file: Fat32.cpp
 *
 *   @date: Jun 29, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "Fat32.h"
#include "kstd.h"

using kstd::vector;
using kstd::string;
namespace filesystem {


Fat32::Fat32(drivers::AtaDevice& hdd) : hdd(hdd) {
}

BiosParameterBlock32 Fat32::read_bios_block(u32 partition_offset) {
    BiosParameterBlock32 bpb;
    hdd.read28(partition_offset, &bpb, sizeof(bpb));
    return bpb;
}

void Fat32::print_bios_block(const BiosParameterBlock32& bpb, ScreenPrinter& printer) {
    string label = trim(bpb.volume_label, sizeof(bpb.volume_label));
    string software = trim (bpb.software_name, sizeof(bpb.software_name));
    string fat_type_label = trim(bpb.fat_type_label, sizeof(bpb.fat_type_label));
    printer.format("Volume label: %, Software: %, Fat type: %\n", label, software, fat_type_label);
}

vector<DirectoryEntry32> Fat32::read_root_directory(u32 partition_offset) {
    BiosParameterBlock32 bpb = read_bios_block(partition_offset);
    u32 fat_start = partition_offset + bpb.reserved_sectors;
    u32 fat_size = bpb.table_size;
    u32 data_start = fat_start + fat_size * bpb.fat_copies;
    u32 root_start = data_start + bpb.sectors_per_cluster * (bpb.root_cluster_count  -2); // -2 for offset
    vector<DirectoryEntry32> result (16);
    hdd.read28(root_start, result.data(), sizeof(DirectoryEntry32) * 16);
    return result;
}

void Fat32::print_directory_entries(const BiosParameterBlock32& bpb, u32 partition_offset, const vector<DirectoryEntry32>& entries, ScreenPrinter& printer) {

    u32 fat_start = partition_offset + bpb.reserved_sectors;
    u32 fat_size = bpb.table_size;
    u32 data_start = fat_start + fat_size * bpb.fat_copies;

    printer.format("Files:\n");

    for (const auto& e : entries) {
        if (e.name[0] == '\0')
            break; // no more entries

        if ((e.attributes & ATTR_ILLEGAL) == ATTR_ILLEGAL)
            continue; // illegal attribute; special purpose metadata entry; skip

        string name = trim(e.name, sizeof(e.name));
        string ext = trim(e.ext, sizeof(e.ext));
        string full_name = ext.empty() ? name : name + "." + ext;

        if ((e.attributes & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
            printer.format("[%]\n", full_name);
            continue;
        }

        if (e.size  == 0) {
            printer.format("% - [EMPTY]\n", full_name);
            continue;
        }
        u32 file_cluster = e.first_cluster_hi << 16 | e.first_cluster_lo;
        u32 file_sector = data_start +  bpb.sectors_per_cluster * (file_cluster - 2);
        u8 buff[512];
        hdd.read28(file_sector, buff, sizeof(buff));
        buff[e.size-1] = '\0';
        printer.format("% - \"%\"\n", full_name, (char*)buff);
    }
    printer.format("\n");
}

string Fat32::trim(const u8 *in, u16 len) {
    string s((const char*)in, len);
    auto pred = [] (u8 c) { return (c < 33 || c > 127); };
    auto last = std::find_if(s.begin(), s.end(), pred);
    return string (s.begin(), last);
}
} /* namespace cpuexceptions */
