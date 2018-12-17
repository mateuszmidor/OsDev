/**
 *   @file: VfsEntry.h
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSENTRY_H_
#define SRC_FILESYSTEM_VFSENTRY_H_

#include "types.h"
#include "UnixPath.h"
#include "EntryState.h"
#include "../CommonStructs.h"

namespace filesystem {


/**
 * @brief   This class is a common interface for Virtual File System entry (in general: file or directory; mountpoint is a superset of directory).
 * @note    Default implementation returns invalid operation. Proper implementations of file/directory/mountpoint
 *          shall overwrite related methods. This way we shorten both amount of typing and flatten type hierarchy.
 */
class VfsEntry {
protected:
    constexpr static auto SUCCESS_OP    {middlespace::ErrorCode::EC_OK};
    constexpr static auto INVALID_OP    {middlespace::ErrorCode::EC_PERM};

public:
    VfsEntry() {};
    virtual ~VfsEntry() {}

    // [common interface]
    virtual const cstd::string& get_name() const = 0;
    virtual utils::SyscallResult<void> set_name(const cstd::string& name)                                   { return {INVALID_OP};  }
    virtual VfsEntryType get_type() const = 0;
    virtual bool is_mountpoint() const                                                                      { return false;         }
    virtual utils::SyscallResult<EntryState*> open()                                                        { return {nullptr};     }
    virtual utils::SyscallResult<void> close(EntryState* s)                                                 { delete s; return {SUCCESS_OP};  }

    // [file interface]
    virtual utils::SyscallResult<u64> get_size() const                                                      { return {INVALID_OP};  }
    virtual utils::SyscallResult<u64> read(EntryState* state, void* data, u32 count)                        { return {INVALID_OP};  }
    virtual utils::SyscallResult<u64> write(EntryState* state, const void* data, u32 count)                 { return {INVALID_OP};  }
    virtual utils::SyscallResult<void> seek(EntryState* state, u32 new_position)                            { return {INVALID_OP};  }
    virtual utils::SyscallResult<void> truncate(EntryState* state, u32 new_size)                            { return {INVALID_OP};  }
    virtual utils::SyscallResult<u64> get_position(EntryState* state) const                                 { return {INVALID_OP};  }

    // [directory interface]
    virtual utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& path)                               { return {INVALID_OP};  }
    virtual utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry)                   { return {INVALID_OP};  }

    // [mount point interface]
    virtual utils::SyscallResult<VfsEntryPtr> create_entry(const UnixPath& path, bool is_directory)         { return {INVALID_OP};  }
    virtual utils::SyscallResult<void> delete_entry(const UnixPath& path)                                   { return {INVALID_OP};  }
    virtual utils::SyscallResult<void> move_entry(const UnixPath& from, const UnixPath& to)                 { return {INVALID_OP};  }
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSENTRY_H_ */
