/**
 *   @file: VfsDateEntry.h
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_
#define SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_

#include "VfsFileEntry.h"
#include "Port.h"

namespace filesystem {

/**
 * @brief   This class exposes system date and time as virtual filesystem entry
 */
class VfsDateEntry: public VfsFileEntry {
public:
    // [common interface]
    bool open() override;
    void close() override;
    const cstd::string& get_name() const    { return name; }

    // [file interface]
    u32 get_size() const override;
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override    { return false; }
    bool truncate(u32 new_size) override    { return false; }
    u32 get_position() const override       { return 0; }


private:
    cstd::string get_date_time() const;
    u8 read_byte(u8 offset) const;
    u8 to_bin(u8 bcd) const;
    const cstd::string      name            {"date"};
    bool                    is_open         {false};
    hardware::Port8bitSlow  address         {0x70};
    hardware::Port8bitSlow  data            {0x71};
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_PROCFS_VFSDATEENTRY_H_ */
