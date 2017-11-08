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

using namespace memory;
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
 * @return  Newly added task id or 0 if max task count is reached
 */
u32 TaskManager::add_task(const Task& task) {
    if (running_queue.count() >= MAX_TASKS)
        return 0;

    u32 tid = current_task_id;
    Task* t = new Task(task);
    t->prepare(tid, TaskManager::on_task_finished);
    running_queue.push_front(t);

    current_task_id++;

    return tid;
}

Task& TaskManager::get_current_task() {
    return *current_task;
}

const TaskList& TaskManager::get_tasks() const {
    return running_queue;
}

u16 TaskManager::get_num_tasks() const {
    return running_queue.count();
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
    // nothing to schedule?
    if (running_queue.count() == 0)
        return cpu_state;

    // store cpu state in current task
    if (current_task) {
        if (current_task->is_user_space) {
            current_task->cpu_state = (CpuState*) (cpu_state->rsp - sizeof(CpuState)); // allocate cpu state on the current task stack
            *(current_task->cpu_state) = *cpu_state; // copy cpu state from kernel stack to task stack
        }
        else
            current_task->cpu_state = cpu_state;     // cpu state is already allocated and stored on task stack, just remember the pointer
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
    close_files(*current_task);
    release_address_space(*current_task);

    running_queue.remove(current_task);
    delete current_task;
    current_task = nullptr;

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

/**
 * @brief   Close all the files open for "task"
 */
void TaskManager::close_files(Task& task) {
    for (auto& f : task.files)
        if (f)
            f->close();
}

/**
 * @brief   Release physical memory frames allocated for "task"
 */
void TaskManager::release_address_space(Task& task) {
    if (task.pml4_phys_addr == 0)
        return;

    if (task.pml4_phys_addr == PageTables::get_kernel_pml4_phys_addr()) // dont remove kernel address space :)
        return;

    u64* pde_virt_addr =  PageTables::get_page_for_virt_address(0, task.pml4_phys_addr); // user task virtual address space starts at virt address 0

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
    while (running_queue.contains(task_id))
        Task::yield();
}

/**
 * @brief   Take the current task out of the running queue and put it on "list"
 */
void TaskManager::dequeue_current_task(TaskList& list) {
    next_task = current_task->next;
    running_queue.remove(current_task);
    list.push_front(current_task);
}

/**
 * @brief   Put the "task" back on running queue
 * @note    IT MUST HAVE BEEN FIRST ADDED AND INITIALIZED WITH "add_task"
 */
void TaskManager::enqueue_task_back(Task* task) {
    running_queue.push_front(task);
}

/**
 * @brief   Choose next task to run and load its page table level4 into cr3
 */
CpuState* TaskManager::pick_next_task_and_load_address_space() {
    if (!next_task)
        next_task = running_queue.front();
    current_task = next_task;
    next_task = next_task->next;

    u64 pml4_physical_address = current_task->pml4_phys_addr;
    PageTables::load_address_space(pml4_physical_address);

    return current_task->cpu_state;
}
} // namespace multitasking {
