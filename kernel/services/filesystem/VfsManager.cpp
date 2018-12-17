/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "VfsManager.h"
#include "VfsOpenEntry.h"
#include "run_this_guy.h"

using namespace utils;
using namespace middlespace;

namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install vfs root "/" directory
 */
void VfsManager::install() {
    tree.install();
}

utils::SyscallResult<OpenEntryPtr> VfsManager::open(const UnixPath& path) {
    auto entry = tree.get_or_bring_entry_to_cache(path);
    if (!entry)
        return {ErrorCode::EC_NOENT};

    auto open_result = entry->open();
    if (!open_result)
        return {open_result.ec};

    auto state = open_result.value;

    const auto on_destroy = rtg::run_this_guy(&VfsTree::release_cached, tree);

    return {std::make_shared<VfsOpenEntry>(entry, state, on_destroy)};
}

utils::SyscallResult<void> VfsManager::attach(const UnixPath& path, const VfsEntryPtr& entry) {
    return tree.attach(entry, path);
}

utils::SyscallResult<void> VfsManager::create(const UnixPath& path, bool is_directory) {
    return tree.create(path, is_directory);
}

utils::SyscallResult<void> VfsManager::remove(const UnixPath& path) {
    return tree.remove(path);
}

utils::SyscallResult<void> VfsManager::copy(const UnixPath& path_from, const UnixPath& path_to) {
    return tree.copy(path_from, path_to);
}

utils::SyscallResult<void> VfsManager::move(const UnixPath& path_from, const UnixPath& path_to) {
    return tree.move(path_from, path_to);
}

bool VfsManager::exists(const UnixPath& path) const {
    return tree.exists(path);
}

} /* namespace filesystem */
