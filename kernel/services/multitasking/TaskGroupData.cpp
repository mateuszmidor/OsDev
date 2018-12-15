/**
 *   @file: TaskGroupData.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskGroupData.h"
#include "Requests.h"

namespace multitasking {

TaskGroupData::TaskGroupData(const AddressSpace& as, const cstd::string& cwd, u32 parent_id) :
        address_space(as), cwd(cwd), parent_task_id(parent_id) {
}

TaskGroupData::~TaskGroupData() {
    close_files();
    requests->release_address_space(address_space);
}

/**
 * @brief   Close all the open files in task group
 * @note    Execution context: Interrupt only (on kill_current_task, when Task is destroyed)
 */
void TaskGroupData::close_files() {
    // files closed automatically on "files" array destruction
}

} /* namespace multitasking */
