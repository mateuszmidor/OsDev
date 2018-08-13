/**
 *   @file: EntryState.h
 *
 *   @date: Aug 9, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_
#define KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_

#include <memory>

namespace filesystem {

/**
 * @brief   Base for VfsEntry::open() return type and descendants
 */
struct EntryState {
    virtual ~EntryState() {}
};

using EntryStatePtr = std::unique_ptr<EntryState>;

}




#endif /* KERNEL_SERVICES_FILESYSTEM_BASE_ENTRYSTATE_H_ */
