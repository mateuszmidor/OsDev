/**
 *   @file: Task.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MULTITASKING_TASK_H_
#define SRC_MULTITASKING_TASK_H_

#include <memory>
#include "kstd.h"
#include "CpuState.h"

namespace multitasking {

using TaskEntryPoint = void (*)(u64 arg);
using TaskExitPoint = void (*)();

struct Task {
    Task(TaskEntryPoint entrypoint, kstd::string name = "ktask", u64 arg = 0, bool user_space = false, u64 pml4_phys_addr = 0, u64 stack_addr = 0, u64 stack_size = 0);
    ~Task();
    void prepare(TaskExitPoint exitpoint);
    void wait_until_finished();
    static void idle(u64 arg = 0);
    static void yield();
    static void exit(u64 result_code = 0);

    volatile bool is_terminated;
    TaskEntryPoint entrypoint;
    kstd::string name;
    u64 arg;
    bool is_user_space;
    u64 pml4_phys_addr;
    u64 stack_addr; // virtual address in task address space (pml4_phys_addr)
    u64 stack_size;
    hardware::CpuState* cpu_state;
};

struct TaskEpilogue {
    u64 rip;    // rip cpu register value for retq instruction on task function exit
} __attribute__((packed));

using TaskPtr = std::shared_ptr<Task>;

} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASK_H_ */
