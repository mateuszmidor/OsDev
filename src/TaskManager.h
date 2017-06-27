/**
 *   @file: TaskManager.h
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_TASKMANAGER_H_
#define SRC_TASKMANAGER_H_

#include <array>
#include "CpuState.h"
#include "kstd.h"

using TaskEntryPoint = void (*)();

struct Task {
    Task(TaskEntryPoint entrypoint, kstd::string name = "ktask");
    kstd::string name;
    u8  stack[4096];
    cpu::CpuState* cpu_state;
};

struct TaskEpilogue {
    u64 rip;    // rip cpu register value for retq instruction on task function exit
} __attribute__((packed));

class TaskManager {
public:
    static TaskManager& instance();
    TaskManager operator=(const TaskManager&) = delete;
    TaskManager operator=(TaskManager&&) = delete;

    static void on_task_exit();
    bool add_task(Task* task);
    Task const * get_current_task() const;
    cpu::CpuState* schedule(cpu::CpuState* cpu_state);
    cpu::CpuState* kill_current_task();

private:
    TaskManager() {}
    static TaskManager _instance;

    std::array<Task*, 256> tasks; // TODO: make it array of shared_ptr<Task>
    u16 num_tasks       =  0;
    s16 current_task    = -1;
};

#endif /* SRC_TASKMANAGER_H_ */
