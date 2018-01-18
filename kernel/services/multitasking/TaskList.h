/**
 *   @file: TaskList.h
 *
 *   @date: Nov 8, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_TASKLIST_H_
#define KERNEL_MULTITASKING_TASKLIST_H_

#include "List.h"

namespace multitasking {

// forward declaration
class Task;


/**
 * @brief   This class represents a list of Tasks to hold the running/waiting tasks
 */
class TaskList : public cstd::List<Task*> {
public:
    bool contains(u32 task_id) const;
    Task* get_by_tid(u32 task_id);
};

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_TASKLIST_H_ */
