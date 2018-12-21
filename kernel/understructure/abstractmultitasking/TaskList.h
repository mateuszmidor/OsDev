#ifndef TASK_LIST
#define TASK_LIST

#include "List.h"

namespace multitasking {

/**
 * @brief   Task represents single CPU-scheduable execution unit
 */
class Task;

/**
 * @brief   This class represents a list of Tasks to hold the running/waiting tasks
 */

class TaskList : public cstd::List<Task*> {
};

} /* namespace multitasking */

#endif