/**
 *   @file: VfsRamFifoEntry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_
#define SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_

#include "VfsFileEntry.h"
#include "TaskManager.h"

namespace filesystem {

/**
 * @brief   This class is VfsEntry implementation for an in-memory fifo (first in first out) file entry.
 * @note    It acts like FIFO; you always read the head of it, and write the tail, thus get_position() always return 0
 *          It blocks reader if empty, it blocks writer if full.
 */
class VfsRamFifoEntry: public VfsFileEntry {
public:
    VfsRamFifoEntry(const cstd::string& name) : name(name), size(0) {}

    // [common interface]
    const cstd::string& get_name() const override   { return name; }

    // [file interface]
    u32 get_size() const override                   { return size; }
    s64 read(void* data, u32 count) override;
    s64 write(const void* data, u32 count) override;
    bool seek(u32 new_position) override            { return false; }
    bool truncate(u32 new_size) override;
    u32 get_position() const override               { return 0; }

private:
    static constexpr u32        BUFF_SIZE           {512};
    const cstd::string          name;
    u8                          buff[BUFF_SIZE];  // in-memory file buffer
    u32                         size;
    multitasking::TaskList      read_wait_list;
    multitasking::TaskList      write_wait_list;

};

using VfsRamFifoEntryPtr = std::shared_ptr<VfsRamFifoEntry>;

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSRAMFIFOENTRY_H_ */
