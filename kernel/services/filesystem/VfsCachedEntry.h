/**
 *   @file: VfsEntryAttachmentDecorator.h
 *
 *   @date: Feb 15, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_VFSCACHEDENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_VFSCACHEDENTRY_H_

#include "VfsEntry.h"
#include "Vector.h"

namespace filesystem {

/**
 * @brief   This class is supposed to be used by vfs cache to hold a VfsEntry.
 *          It's a decorator for VfsEntry that allows attaching extra entries to the entry contents.
 *          Useful for storing in-ram entries like pipes, sockets.
 */

class VfsCachedEntry;
using VfsCachedEntryPtr = std::shared_ptr<VfsCachedEntry>;

class VfsCachedEntry: public VfsEntry {
public:
    VfsCachedEntry(const VfsEntryPtr& e) : e(e) {}
    VfsCachedEntry(VfsEntryPtr&& e) : e(std::move(e)) {}

    // [common interface]
    const cstd::string& get_name() const override                                           { return e->get_name();         }
    utils::SyscallResult<void> set_name(const cstd::string& name) override                  { return e->set_name(name);     }
    VfsEntryType get_type() const override                                                  { return e->get_type();         }
    bool is_mountpoint() const override                                                     { return e->is_mountpoint();    }
    utils::SyscallResult<void> open()  override                                             { return e->open();             }
    utils::SyscallResult<void> close()  override                                            { return e->close();            }

    // [file interface]
    utils::SyscallResult<u64> get_size() const  override                                    { return e->get_size();         }
    utils::SyscallResult<u64> read(void* data, u32 count)  override                         { return e->read(data, count);  }
    utils::SyscallResult<u64> write(const void* data, u32 count)  override                  { return e->write(data, count); }
    utils::SyscallResult<void> seek(u32 new_position)  override                             { return e->seek(new_position); }
    utils::SyscallResult<void> truncate(u32 new_size) override                              { return e->truncate(new_size); }
    utils::SyscallResult<u64> get_position() const  override                                { return e->get_position();     }

    // [directory interface - modified by decorator]
    utils::SyscallResult<VfsEntryPtr> get_entry(const UnixPath& path) override;
    utils::SyscallResult<void> enumerate_entries(const OnVfsEntryFound& on_entry) override;

    // [mount point interface]
    utils::SyscallResult<VfsEntryPtr> create_entry(const UnixPath& path, bool is_directory) { return e->create_entry(path, is_directory);   }
    utils::SyscallResult<void> delete_entry(const UnixPath& path)                           { return e->delete_entry(path);                 }
    utils::SyscallResult<void> move_entry(const UnixPath& from, const UnixPath& to)         { return e->move_entry(from, to);               }

    // [attachments interface]
    bool attach_entry(const VfsEntryPtr& entry);
    bool detach_entry(const cstd::string& name);
    VfsCachedEntryPtr get_attached_entry(const cstd::string& name);
    VfsEntryPtr get_raw_entry() const                                                    { return e; }
    u32 attachment_count() const                                                            { return attached_entries.size(); }
    u32 open_count {0};
private:
    cstd::vector<VfsCachedEntryPtr>::iterator find_attached_entry(const cstd::string& name);

    VfsEntryPtr e;      // decorated entry
    cstd::vector<VfsCachedEntryPtr> attached_entries;    // child entries that are memory-stored, eg PIPE, SOCKET
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_VFSCACHEDENTRY_H_ */
