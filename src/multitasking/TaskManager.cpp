/**
 *   @file: TaskManager.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskManager.h"
#include "PageTables.h"
#include "KernelLog.h"
#include "MemoryManager.h"

using namespace hardware;

namespace multitasking {


TaskManager TaskManager::_instance;

TaskManager& TaskManager::instance() {
    return _instance;
}

/**
 * @brief   When a task finishes its work, it jumps here for the TaskManeger to clear it from the list
 */
void TaskManager::on_task_finished() {
    Task::exit();
}

/**
 * @brief   Add new task to the list of scheduler tasks
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 * @return  Newly added task id
 */
u32 TaskManager::add_task(const Task& task) {
    if (num_tasks == tasks.size())
        return 0;

    u32 tid = current_task_id;
    tasks[num_tasks] = task;
    tasks[num_tasks].prepare(tid, TaskManager::on_task_finished);

    num_tasks++; // first insert, then increment in case task schedule run in between
    current_task_id++;

    return tid;
}

Task& TaskManager::get_current_task() {
    return tasks[current_task];
}

const std::array<Task, TaskManager::MAX_TASKS>& TaskManager::get_tasks() const {
    return tasks;
}

u16 TaskManager::get_num_tasks() const {
    return num_tasks;
}

/**
 * @brief   Task scheduling routine. Simple round robin is used for now
 * @param   cpu_state Current task cpu state.
 * @note    cpu_state IS LOCATED ON THE KERNEL STACK AND NEEDS TO BE COPIED TO THE TASK-SPECIFIC LOCATION IF SWITCHING FROM RING 3 (USER SPACE)!!!
 *          This can be improved by storing cpu_state directly in task-specific location or using per-task kernel stack. To be done later.
 * @note    There must be always at least 1 ("idle") task
 * @note    This is supposed to be run from interrupt handling context
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 */
CpuState* TaskManager::schedule(CpuState* cpu_state) {
    if (num_tasks == 0)
        return cpu_state;

    if (current_task >= 0) {
        if (tasks[current_task].is_user_space) {
            tasks[current_task].cpu_state = (CpuState*) (cpu_state->rsp - sizeof(CpuState)); // allocate cpu state on the current task stack
            *(tasks[current_task].cpu_state) = *cpu_state; // copy cpu state from kernel stack to task stack
        }
        else
            tasks[current_task].cpu_state = cpu_state;     // cpu state is already allocated and stored on task stack, just remember the pointer
    }

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

/**
 * @brief   Remove current task from the list and return new task to be run
 * @note    This is supposed to be run from interrupt handling context
 * @note    Should this be secured from multilevel interrupts in preemptive kernel in the future?
 */
hardware::CpuState* TaskManager::kill_current_task() {
    close_files(tasks[current_task]);
    release_address_space(tasks[current_task]);

    // remove current task from the list
    for (u16 i = current_task; i < num_tasks -1; i++)
        tasks[i] = tasks[i+1];

    tasks[num_tasks-1] = Task();
    num_tasks--;

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

void TaskManager::close_files(Task& task) {
    for (auto& f : task.files)
        if (f)
            f->close();
}

void TaskManager::release_address_space(Task& task) {
    if (task.pml4_phys_addr == 0)
        return;

    if (task.pml4_phys_addr == PageTables::get_kernel_pml4_phys_addr()) // dont remove kernel address space :)
        return;

    u64* pde_virt_addr =  PageTables::get_page_for_virt_address(0, task.pml4_phys_addr); // task virtual address space starts at virt address 0

    // scan 0..1GB of virtual memory
    logging::KernelLog& klog = logging::KernelLog::instance();
    memory::MemoryManager& mngr = memory::MemoryManager::instance();
    klog.format("Rmoving address space of task %\n", task.name);
    for (u32 i = 0; i < 512; i++)
        if (pde_virt_addr[i] != 0) {
            klog.format("  Releasing mem frame\n");
            mngr.free_frames((void*)pde_virt_addr[i], 1); // 1 byte will always correspond to just 1 frame
        }

    // release the page table itself
    klog.format("  Releasing mem frames of PageTables64\n");
    mngr.free_frames((void*)task.pml4_phys_addr, sizeof(PageTables64));
}

/**
 * @brief   Wait for task until it is terminated
 */
void TaskManager::wait(u32 task_id) const {
    bool done = false;

    while (!done) {
        done = true;
        for (u32 i = 0; i < num_tasks; i++) {
            if (tasks[i].task_id == task_id) {
                done = false;
                Task::yield();
            }
        }
    }
}

/**
 * @brief   Choose next task to run and load its page table level4 into cr3
 */
CpuState* TaskManager::pick_next_task_and_load_address_space() {
    current_task = (current_task + 1) % num_tasks;
    u64 pml4_physical_address = tasks[current_task].pml4_phys_addr;
    PageTables::load_address_space(pml4_physical_address);

    return tasks[current_task].cpu_state;
}
} // namespace multitasking {
