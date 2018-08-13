/**
 *   @file: VfsManager.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "VfsManager.h"
#include "run_this_guy.h"

using namespace utils;
using namespace middlespace;

namespace filesystem {

VfsManager VfsManager::_instance;

VfsManager& VfsManager::instance() {
    return _instance;
}

/**
 * @brief   Install vfs elements that use dynamic memory
 */
void VfsManager::install() {
    tree.install();
}

/**
 * @brief   Mount a filesystem under the root "/"
 */
utils::SyscallResult<void> VfsManager::mount(const VfsEntryPtr& mount_point) {
    if (!mount_point->is_mountpoint()) {
        klog.format("VfsManager::mount: provided entry is not a mountpoint\n");
        return {ErrorCode::EC_INVAL};
    }

    return tree.attach(mount_point, "/");
}

utils::SyscallResult<OpenEntry> VfsManager::open(const UnixPath& path) {
    auto entry = tree.get_cached(path);
    if (!entry)
        return {ErrorCode::EC_NOENT};

    auto open_result = entry->open();
    if (!open_result)
        return {open_result.ec};

    auto state = open_result.value;

    const auto on_destroy = rtg::run_this_guy(&VfsTree::release_cached, tree);
    return { {on_destroy, entry, state} };
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
