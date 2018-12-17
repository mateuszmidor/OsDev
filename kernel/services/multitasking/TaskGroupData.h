/**
 *   @file: TaskGroupData.h
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_TASKGROUPDATA_H_
#define KERNEL_MULTITASKING_TASKGROUPDATA_H_

#include <array>
#include <memory>
#include "String.h"
#include "../CommonStructs.h"

namespace multitasking {
 
/**
 * @brief   This class holds data common to a task group (or a PROCESS, if you wish to call it so)
 */
class TaskGroupData {
public:
    TaskGroupData(const AddressSpace& as, const cstd::string& cwd, u32 parent_id);
    ~TaskGroupData();

    cstd::string                                cwd;                // current working directory of task group
    AddressSpace                                address_space;      // address space of task group. This uniquely identifies the group
    std::array<filesystem::OpenEntryPtr, 16>    files;              // list of open files. OpenEntry + EntryState coordinates access to VfsEntry
    u32                                         parent_task_id;     // task that created this task. Root task id = 1
};

using TaskGroupDataPtr = std::shared_ptr<TaskGroupData>;

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_TASKGROUPDATA_H_ */
