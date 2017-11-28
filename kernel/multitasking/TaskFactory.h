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
public:
    /**
     * @brief   Create user space task (runs in protection ring3), with provided address space and stack
     * @param   EntrypointT should be of type TaskEntryPoint0, TaskEntryPoint1 or TaskEntryPoint2
     * @note    Make sure that entrypoint and stack is accessible from that address space,
     *          ie. their virtual addresses are mapped as ring3 accessible
     */
    template <class EntrypointT>
    static Task* make_user_task(EntrypointT entrypoint, const char name[], u64 pml4_phys_addr, u64 stack_addr, u64 stack_size) {
        return new Task(
                        (TaskEntryPoint2)entrypoint,
                        name,
                        0,              // task func arg 1
                        0,              // task func arg 2
                        true,           // user space = true
                        pml4_phys_addr,
                        stack_addr,
                        stack_size,
                        "/"             // current working directory as root
                    );
    }

    /**
     * @brief   Create kernel space task (runs in protection ring0), with kernel address space and default stack
     * @param   EntrypointT should be of type TaskEntryPoint0, TaskEntryPoint1 or TaskEntryPoint2
     */
    template <class EntrypointT>
    static Task* make_kernel_task(EntrypointT entrypoint, const char name[]) {
        return new Task(
                        (TaskEntryPoint2)entrypoint,
                        name,
                        0,              // task func arg 1
                        0,              // task func arg 2
                        false,          // user space = false
                        0,              // use kernel address space
                        0,              // create default stack...
                        0,              // ...of default size
                        "/"             // current working directory as root
                    );
    }

    /**
     * @brief   Makes a Task object from a class that exposes Constructor(u64 arg) and void run()
     */
//    template <class T>
//    static Task make_kernel_task(const kstd::string& name, u64 arg = 0) {
//        return Task::make_kernel_task(make_<T>, name.c_str()).set_arg1(arg);
//    }

private:
    template <class T>
    static void make_(u64 arg) {
        T task(arg);
        task.run();
    }
};

} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASKFACTORY_H_ */
