/**
 *   @file: TaskFactory.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MULTITASKING_TASKFACTORY_H_
#define SRC_MULTITASKING_TASKFACTORY_H_

#include "Task.h"

namespace multitasking {

class TaskFactory {
private:
    static TaskGroupData    kernel_task_group;
    static TaskGroupDataPtr kernel_task_group_ptr;

public:
    /**
     * @brief   Create kernel space task (runs in protection ring0), with kernel address space and default stack
     * @param   EntrypointT should be of type TaskEntryPoint0, TaskEntryPoint1 or TaskEntryPoint2
     */
    template <class EntrypointT>
    static Task* make_kernel_task(EntrypointT entrypoint, const char name[]) {
        // this init is done only once when "init" task is being created (thus no preemptiom possible)
        if (!kernel_task_group_ptr)
            kernel_task_group_ptr.reset(&kernel_task_group);

        u64 stack_size = Task::DEFAULT_STACK_SIZE;
        u64 stack_addr = (u64)new char[stack_size];

        return new Task(
                        (TaskEntryPoint2)entrypoint,
                        name,                   // task name
                        0,                      // task func arg 1
                        0,                      // task func arg 2
                        false,                  // user space = false
                        stack_addr,
                        stack_size,
                        kernel_task_group_ptr   // all kernel tasks belong to one group
                    );
    }

    /**
     * @brief   Create a task that shares the resources with "src" task.
     *          To be used to add threads within same process.
     * @param   EntrypointT should be of type TaskEntryPoint0, TaskEntryPoint1 or TaskEntryPoint2
     */
    template <class EntrypointT>
    static Task* make_lightweight_task(const Task& src, EntrypointT entrypoint, const char name[], u64 stack_size) {
        TaskGroupDataPtr task_group_data = src.task_group_data;
        u64 stack_addr = (u64)task_group_data->alloc_static(stack_size);

        return new Task(
                        (TaskEntryPoint2)entrypoint,
                        name,                   // task name
                        0,                      // task func arg 1
                        0,                      // task func arg 2
                        src.is_user_space,      // execution space same as source task
                        stack_addr,
                        stack_size,
                        task_group_data         // group same assource task
                    );
    }

    /**
     * @brief   Create fake task that represents kernel main until entering multitasking
     */
    static Task make_boot_stub_task() {
        return Task((TaskEntryPoint2)0, "boot", 0, 0, false, 0, 0, TaskGroupDataPtr {});
    }

};

} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASKFACTORY_H_ */
