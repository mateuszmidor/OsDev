/**
 *   @file: VfsOpenEntry.h
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_

#include "VfsCachedEntry.h"
#include "OpenEntry.h"

namespace filesystem {

/**
 * @brief   This struct holds VFS Entry and it's individual open instance state and uses RAII to close when the time comes
 */

class VfsOpenEntry : public OpenEntry {
    using OnDestroy = std::function<void(const VfsCachedEntryPtr&)>;

public:
    VfsOpenEntry(VfsCachedEntryPtr e = {}, EntryState* s = {}, const OnDestroy& on_destroy = {});
    ~VfsOpenEntry();
    VfsOpenEntry operator=(const VfsOpenEntry&) = delete;
    VfsOpenEntry(const VfsOpenEntry&) = delete;
    VfsOpenEntry(VfsOpenEntry&& e);
    VfsOpenEntry& operator=(VfsOpenEntry&&);

    // [common interface]
    VfsEntryType get_type() const override                                          { return entry->get_type();         }

    // [file interface]
    utils::SyscallResult<u64> get_size() const override                             { return entry->get_size();                 }
    utils::SyscallResult<u64> read(void* data, u32 count) override                  { return entry->read(state, data, count);  }
    utils::SyscallResult<u64> write(const void* data, u32 count) override           { return entry->write(state, data, count); }
    utils::SyscallResult<void> seek(u32 new_position) override                      { return entry->seek(state, new_position); }
    utils::SyscallResult<void> truncate(u32 new_size) override                      { return entry->truncate(state, new_size); }
    utils::SyscallResult<u64> get_position() const override                         { return entry->get_position(state);       }

    // [directory interface]
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) override  { return entry->enumerate_entries(on_entry);    }

public:
    VfsCachedEntryPtr   entry   {nullptr};
    EntryState*         state   {nullptr};

private:
    OnDestroy  on_destroy;
    void dispose();
};



} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_ */
