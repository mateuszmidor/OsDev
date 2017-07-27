/*
 * ps.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: mateusz
 */

#include "ps.h"
#include "TaskManager.h"

using namespace multitasking;
namespace cmds {

void ps::run() {
    TaskManager& task_manager = TaskManager::instance();
    const auto& tasks = task_manager.get_tasks();
    for (u16 i = 0; i < task_manager.get_num_tasks(); i++)
        env->printer->format("% %\n", i, tasks[i]->name);
}
} /* namespace cmds */
