/**
 *   @file: TaskFactory.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskFactory.h"

namespace multitasking {

TaskGroupData    TaskFactory::kernel_task_group({}, "/", 1); // no address space at the beginning
TaskGroupDataPtr TaskFactory::kernel_task_group_ptr;

} /* namespace multitasking */
