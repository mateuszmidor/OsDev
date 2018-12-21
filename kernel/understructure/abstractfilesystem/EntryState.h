/**
 *   @file: EntryState.h
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_
#define KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_

namespace filesystem {

/**
 * @brief   Base for VfsEntry::open() return type and descendants, it will hold current read/write position
 */
struct EntryState {
    virtual ~EntryState() {}
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_ */
