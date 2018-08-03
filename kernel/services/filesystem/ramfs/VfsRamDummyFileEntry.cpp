/**
 *   @file: VfsRamDummyFileEntry.cpp
 *
 *   @date: Aug 2, 2018
 * @author: Mateusz Midor
 */

#include "VfsRamDummyFileEntry.h"

namespace filesystem {

utils::SyscallResult<void> VfsRamDummyFileEntry::set_name(const cstd::string& name) {
    this->name = name;
    return {middlespace::ErrorCode::EC_OK};
}

} /* namespace filesystem */
