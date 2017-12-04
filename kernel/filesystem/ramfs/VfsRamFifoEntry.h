/**
 *   @file: VfsRamFifoEntry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_
#define SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_

#include "VfsEntry.h"
#include "TaskManager.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory fifo (first in first out) file entry.
 * @note    It acts like FIFO; you always read the head of it, and write the tail, thus get_position() always return 0
 */
class VfsRamFifoEntry: public VfsEntry {
private:
    static constexpr u32        BUFF_SIZE   = 512;
    u8                          buff[BUFF_SIZE];  // in-memory file buffer
    kstd::string                name;
    u32                         size;
    multitasking::TaskList      read_wait_list;
    multitasking::TaskList      write_wait_list;
public:
    VfsRamFifoEntry(const kstd::string& name) : name(name), size(0) {}

    // [common interface]
    bool open() override                            { return true; /* no initialization to do here */ }
    void close() override                           {};
    bool is_directory() const override              { return false; }
    const kstd::string& get_name() const override   { return name; }

    // [file interface]
    u32 get_size() const override                   { return size; }
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override            { return false; }
    bool truncate(u32 new_size) override            { return false; }
    u32 get_position() const override               { return 0; }

    // [directory interface]
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override { return VfsEnumerateResult::ENUMERATION_FAILED; }
};

using VfsRamFifoEntryPtr = std::shared_ptr<VfsRamFifoEntry>;

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_ */
