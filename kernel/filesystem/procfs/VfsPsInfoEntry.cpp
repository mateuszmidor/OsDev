/**
 *   @file: VfsPsInfoEntry.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include <errno.h>
#include "VfsPsInfoEntry.h"
#include "TaskManager.h"

using namespace multitasking;
namespace filesystem {

bool VfsPsInfoEntry::open() {
    if (is_open)
        return false;

    is_open = true;
    return true;
}

void VfsPsInfoEntry::close() {
    is_open = false;
}

/**
 * @brief   The size of the info is not known until the info string is built
 */
u32 VfsPsInfoEntry::get_size() const {
    return 0;
}

/**
 * @brief   Read the last "count" info bytes
 * @return  Num of read bytes
 */
s64 VfsPsInfoEntry::read(void* data, u32 count) {
    if (!is_open) {
        return 0;
    }

    if (count == 0)
        return 0;

    TaskManager& task_manager = TaskManager::instance();
    kstd::string info;
    u32 i = 0;
    const TaskList& tasks = task_manager.get_tasks();
    for (const Task* task : tasks) {
        info += kstd::StringUtils::format("%. % [%], tid %\n", i, task->name, task->is_user_space ? "User" : "Kernel", task->task_id);
        i++;
    }

    if (info.empty())
        return 0;

    u32 read_start = kstd::max((s64)info.length() - count, 0);
    u32 num_bytes_to_read = kstd::min(count, info.length());

    memcpy(data, info.c_str() + read_start, num_bytes_to_read);

    close();
    return num_bytes_to_read;
}

s64 VfsPsInfoEntry::write(const void* data, u32 count) {
    return -EPERM;
}
} /* namespace filesystem */
