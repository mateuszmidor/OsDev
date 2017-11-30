/**
 *   @file: Task.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MULTITASKING_TASK_H_
#define SRC_MULTITASKING_TASK_H_

#include "CpuState.h"
#include "TaskList.h"
#include "TaskGroupData.h"

namespace multitasking {

using TaskEntryPoint0 = void (*)();
using TaskEntryPoint1 = void (*)(u64 arg1);
using TaskEntryPoint2 = void (*)(u64 arg1, u64 arg2);
using TaskExitPoint = void (*)();


struct TaskEpilogue {
    u64 rip;    // rip cpu register value for retq instruction on task function exit
} __attribute__((packed));

struct Task {
    // Task cannot be simply copied as it holds some shared resources (task group data)
    Task(const Task& other) = delete;
    Task& operator=(const Task& other) = delete;

    // But can be moved
    Task& operator=(Task&& other) = default;
    Task(Task&& other) = default;

    // Or created from scratch
    Task(TaskEntryPoint2 entrypoint, const char name[], u64 arg1, u64 arg2, bool user_space, u64 stack_addr, u64 stack_size, TaskGroupDataPtr task_group_data);
    ~Task();
    void prepare(u32 tid, TaskExitPoint exitpoint);


    static void idle();
    static void yield();
    static void exit(u64 result_code = 0);
    static void exit_group(u64 result_code = 0);


    template <class T>
    Task* set_arg1(T value) { arg1 = (u64) value; return this; }

    template <class T>
    Task* set_arg2(T value) { arg2 = (u64) value; return this; }



    u32                 task_id;            // set by TaskManager when first adding the task
    TaskEntryPoint2     entrypoint;         // covers TaskEntryPoint0 and TaskEntryPoint1
    kstd::string        name;
    u64                 arg1;
    u64                 arg2;
    bool                is_user_space;
    u64                 stack_addr;
    u64                 stack_size;
    hardware::CpuState* cpu_state;
    TaskList            wait_queue;         // list of tasks waiting for this task to finish

    TaskGroupDataPtr    task_group_data;    // task group where this task belong


    static const u64    DEFAULT_STACK_SIZE = 2 * 4096;

};


} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASK_H_ */
