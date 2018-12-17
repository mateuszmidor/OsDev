/**
 *   @file: TaskGroupData.cpp
 *
 *   @date: Nov 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskGroupData.h"
#include "Requests.h"

namespace multitasking {

TaskGroupData::TaskGroupData(const memory::AddressSpace& as, const cstd::string& cwd, u32 parent_id) :
        address_space(as), cwd(cwd), parent_task_id(parent_id) {
}

TaskGroupData::~TaskGroupData() {
    requests->release_address_space(address_space);
}

} /* namespace multitasking */
