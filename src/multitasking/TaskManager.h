/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include <array>
#include <memory>
#include "CpuState.h"
#include "kstd.h"

namespace multitasking {

using TaskEntryPoint = void (*)(u64 arg);
struct Task {
    Task(TaskEntryPoint entrypoint, kstd::string name = "ktask", u64 arg = 0);
    static void idle(u64 arg = 0);
    static void yield();
    static void exit();

    kstd::string name;
    u8  stack[2*4096];
    hardware::CpuState* cpu_state;
};

struct TaskEpilogue {
    u64 rip;    // rip cpu register value for retq instruction on task function exit
} __attribute__((packed));

using TaskPtr = std::shared_ptr<Task>;
class TaskManager {
public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    static void on_task_finished();
    bool add_task(TaskPtr task);
    TaskPtr get_current_task() const;
    u16 get_num_tasks() const;
    hardware::CpuState* schedule(hardware::CpuState* cpu_state);
    hardware::CpuState* kill_current_task();

private:
    TaskManager() {}
    static TaskManager _instance;

    std::array<TaskPtr, 256> tasks;
    u16 num_tasks       =  0;
    s16 current_task    = -1;
};

}

#endif /* SRC_TASKMANAGER_H_ */
