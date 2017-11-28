/**
 *   @file: Task.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MULTITASKING_TASK_H_
#define SRC_MULTITASKING_TASK_H_

#include <array>
#include "kstd.h"
#include "CpuState.h"
#include "TaskList.h"
#include "VfsEntry.h"

namespace multitasking {

using TaskEntryPoint0 = void (*)();
using TaskEntryPoint1 = void (*)(u64 arg1);
using TaskEntryPoint2 = void (*)(u64 arg1, u64 arg2);
using TaskExitPoint = void (*)();

struct TaskEpilogue {
    u64 rip;    // rip cpu register value for retq instruction on task function exit
} __attribute__((packed));

struct Task {
    Task();
    Task(TaskEntryPoint2 entrypoint, const char name[], u64 arg1, u64 arg2, bool user_space, u64 pml4_phys_addr, u64 stack_addr, u64 stack_size, const char cwd[]);
    void prepare(u32 tid, TaskExitPoint exitpoint);


    static void idle();
    static void yield();
    static void exit(u64 result_code = 0);


    template <class T>
    Task* set_arg1(T value) { arg1 = (u64) value; return this; }

    template <class T>
    Task* set_arg2(T value) { arg2 = (u64) value; return this; }


    static const u64    DEFAULT_STACK_SIZE = 2 * 4096;
    u32                 task_id;        // set by TaskManager when first adding the task
    TaskEntryPoint2     entrypoint;     // covers TaskEntryPoint0 and TaskEntryPoint1
    kstd::string        name;
    kstd::string        cwd;            // current working directory
    u64                 arg1;
    u64                 arg2;
    bool                is_user_space;
    u64                 pml4_phys_addr;
    u64                 stack_addr;
    u64                 stack_size;
    hardware::CpuState* cpu_state;

    std::array<filesystem::VfsEntryPtr, 16> files;  // per-task list of open files. This should be later made per-process. TODO: concurrent access to the same file. How?

    TaskList            wait_queue; // list of tasks waiting for this task to finish
};


} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASK_H_ */
