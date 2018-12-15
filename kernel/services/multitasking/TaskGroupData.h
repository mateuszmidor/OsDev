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
#include "OpenEntry.h"
#include "../CommonStructs.h"

namespace multitasking {
 
/**
 * @brief   This class holds data common to a task group (or a PROCESS, if you wish to call it so)
 */
class TaskGroupData {
public:
    TaskGroupData(const AddressSpace& as, const cstd::string& cwd, u32 parent_id);
    ~TaskGroupData();

    cstd::string                            cwd;                // current working directory of task group
    AddressSpace                            address_space;      // address space of task group. This uniquely identifies the group
    std::array<filesystem::OpenEntry, 16>   files;              // list of open files. TODO: concurrent access to the same file. How?
    u32                                     parent_task_id;

private:
    void close_files();
};

using TaskGroupDataPtr = std::shared_ptr<TaskGroupData>;

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_TASKGROUPDATA_H_ */
