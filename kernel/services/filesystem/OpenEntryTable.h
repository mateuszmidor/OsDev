/**
 *   @file: OpenEntryTable.h
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_
#define KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_

#include "Vector.h"
#include "Optional.h"
#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   Represents a file descriptor that is global throughout the system
 */
using GlobalFileDescriptor = u32;

/**
 * @brief   This struct holds VFS Entry and it's individual open instance state
 */
struct OpenEntry {
    VfsEntryPtr entry   {nullptr};
    EntryState* state   {nullptr};
};

class OpenEntryTable {
public:
    void install();
    cstd::Optional<GlobalFileDescriptor> allocate(const VfsEntryPtr& e, EntryState* state);
    void deallocate(GlobalFileDescriptor fd);
private:
    cstd::Optional<GlobalFileDescriptor> find_free_fd() const;
    cstd::vector<OpenEntry> open_entries;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_OPENENTRYTABLE_H_ */
