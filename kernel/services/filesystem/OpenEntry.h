/**
 *   @file: OpenEntry.h
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_

#include "VfsCachedEntry.h"
#include "EntryState.h"


namespace filesystem {

/**
 * @brief   This struct holds VFS Entry and it's individual open instance state and uses RAII to close when the time comes
 */

class OpenEntry {
    using OnDestroy = std::function<void(const VfsCachedEntryPtr&)>;

public:
    OpenEntry(VfsCachedEntryPtr e = {}, EntryState* s = {}, const OnDestroy& on_destroy = {});
    ~OpenEntry();
    OpenEntry operator=(const OpenEntry&) = delete;
    OpenEntry(const OpenEntry&) = delete;
    OpenEntry(OpenEntry&& e);
    OpenEntry& operator=(OpenEntry&&);
    operator bool() const                                                           { return (bool)entry;               }
    bool operator!() const                                                          { return !entry;                    }

    // [common interface]
    VfsEntryType get_type() const                                                   { return entry->get_type();         }

    // [file interface]
    utils::SyscallResult<u64> get_size() const                                      { return entry->get_size();                 }
    utils::SyscallResult<u64> read(void* data, u32 count)                           { return entry->read(state, data, count);  }
    utils::SyscallResult<u64> write(const void* data, u32 count)                    { return entry->write(state, data, count); }
    utils::SyscallResult<void> seek(u32 new_position)                               { return entry->seek(state, new_position); }
    utils::SyscallResult<void> truncate(u32 new_size)                               { return entry->truncate(state, new_size); }
    utils::SyscallResult<u64> get_position() const                                  { return entry->get_position(state);       }

    // [directory interface]
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry)   { return entry->enumerate_entries(on_entry);    }

public:
    VfsCachedEntryPtr   entry;//{nullptr}; - std::shared_ptr gets born as null by default
    EntryState*         state   {nullptr};

private:
    OnDestroy  on_destroy;
    void dispose();
};



} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_OPENENTRY_H_ */
