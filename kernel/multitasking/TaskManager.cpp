/**
 *   @file: TaskManager.cpp
 *
 *   @date: Jun 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageTables.h"
#include "KernelLog.h"
#include "MemoryManager.h"
#include "TimeManager.h"
#include "KLockGuard.h"

using namespace ktime;
using namespace memory;
using namespace hardware;
namespace multitasking {


TaskManager TaskManager::_instance;

TaskManager& TaskManager::instance() {
    return _instance;
}

/**
 * @brief   When a task finishes its work, it jumps here for the TaskManeger to clear it from the list
 * @note    Execution context: Task only; be careful with possible reschedule during execution of this method
 */
void TaskManager::on_task_finished() {
    // KLockGuard lock;    // reschedule not harmful here as no shared data is being modified
    Task::exit();
}

/**
 * @brief   Add new task to the list of scheduler tasks
 * @note    TaskManager takes ownership of "task" pointer
 * @return  Newly added task id or 0 if max task count is reached
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 * @note    First task to be added must be the "idle" task
 */
u32 TaskManager::add_task(Task* task) {
    KLockGuard lock;    // prevent reschedule

    u32 tid = next_task_id;
    task->prepare(tid, TaskManager::on_task_finished);
    if (!scheduler.add(task)) {
        delete task;
        return 0;
    }

    next_task_id++;
    return tid;
}

/**
 * @brief   Add task that reuse tid and address space of current task, exit current task
 * @note    TaskManager takes ownership of "task" pointer
 * @note    As a result, current_task will exit and die, its tid and address space is taken over by "task"
 * @note    Execution context: Task only; be careful with possible reschedule during execution of this method
 */
void TaskManager::replace_current_task(Task* task) {
    KLockGuard lock;    // prevent reschedule, especially between "current_task->pml4_phys_addr = 0" and "Task::exit()"

    const Task& current_task = get_current_task();
    u32 tid = current_task.task_id;
    task->prepare(tid, TaskManager::on_task_finished);
    if (!scheduler.add(task)) {
        delete task;
        return;
    }

    Task::exit();  // current task dies
}

/**
 * @brief   Get reference to the task currently being executer
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
Task& TaskManager::get_current_task() {
    KLockGuard lock;    // prevent reschedule

    return *scheduler.get_current_task();
}

/**
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
const TaskList& TaskManager::get_tasks() const {
    return scheduler.get_task_list();
}

/**
 * @brief   Sleep current task for at least "millis" milliseconds
 * @note    Execution context: Interrupt only (in 80h)
 */
CpuState* TaskManager::sleep_current_task(CpuState* cpu_state, u64 millis) {
    if (millis > 0) {
        TaskList* tl = new TaskList();
        TimeManager& mngr = TimeManager::instance();
        auto on_expire = [tl] () { TaskManager::instance().unblock_tasks(*tl); delete tl; };
        mngr.emplace(millis, on_expire);
        block_current_task(*tl);
    }

    return schedule(cpu_state);
}

/**
 * @brief   Task scheduling routine. Simple round robin is used for now
 * @param   cpu_state Current task cpu state.
 * @note    Execution context: Interrupt only (Programmable Interval Timer interrupt, int 80h interrupt)
 */
CpuState* TaskManager::schedule(CpuState* cpu_state) {
    // nothing to schedule?
    if (scheduler.count() == 0)
        return cpu_state;

    // store cpu state in current task
    save_current_task_state(cpu_state);

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

/**
 * @brief   Save cpu_state in current task
 * @note    cpu_state IS LOCATED ON THE KERNEL STACK AND NEEDS TO BE COPIED TO THE TASK-SPECIFIC LOCATION IF SWITCHING FROM RING 3 (USER SPACE)!!!
 *          This can be improved by storing cpu_state directly in task-specific location or using per-task kernel stack. To be done later.
 */
void TaskManager::save_current_task_state(CpuState* cpu_state) {
    Task& current_task = get_current_task();
    if (current_task.is_user_space) {
        current_task.cpu_state = (CpuState*) ((cpu_state->rsp - sizeof(CpuState))); // allocate cpu state on the current task stack
        *current_task.cpu_state = *cpu_state; // copy cpu state from kernel stack to task stack
    } else
        current_task.cpu_state = cpu_state; // cpu state is already allocated and stored on kernel task stack, just remember the pointer
}

/**
 * @brief   Remove current task from the list and return new task to be run
 * @note    Execution context: Interrupt only (int 80h)
 */
hardware::CpuState* TaskManager::kill_current_task() {
    Task* current_task = scheduler.get_current_task();

    // enqueue back all waiting tasks and delete the task itself
    wakeup_waitings_and_delete_task(current_task);

    // remove the task from running queue
    scheduler.remove(current_task);

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

/**
 * @brief   Remove all the tasks in current task group effectively killing process and return new task to be run
 * @note    Execution context: Interrupt only (int 80h)
 */
hardware::CpuState* TaskManager::kill_current_task_group() {
    // for each task
    //  if task belong to the group
    //    enqueue back waiting tasks
    //    remove task from the queue
    //    delete task

    const Task& current_task = get_current_task();
    const TaskGroupDataPtr current_task_group = current_task.task_group_data;

    // mark the group as to be terminated so when member tasks get awaken they get immediately deleted
    // NOTE: if a member task stays somewhere on a wait queue, it may hold the process resources forever
    current_task_group->termination_pending = true;

    for (Task* task : scheduler.get_task_list()) {
        if (task->task_group_data != current_task_group)
            continue;

        // enqueue back all waiting tasks and delete the task itself
        wakeup_waitings_and_delete_task(task);

        // remove the task from running queue
        scheduler.remove(task);
    }

    // return next task to switch to
    return pick_next_task_and_load_address_space();
}

/**
 * @brief   Wake up tasks waiting for "task" to finish, then delete the "task" itself
 */
void TaskManager::wakeup_waitings_and_delete_task(Task* task) {
    unblock_tasks(task->wait_queue);
    delete task;
}

/**
 * @brief   Get dummy boot task, just so there is always some TaskManager::get_current_task() result
 */
Task TaskManager::get_boot_task() const {
    return TaskFactory::make_boot_stub_task();
}

/**
 * @brief   Remove current task from running_queue until "task_id" is  terminated
 * @return  True if "task_id" still alive, False if already terminated/not exists
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
bool TaskManager::wait(u32 task_id) {
    KLockGuard lock;    // prevent reschedule

    // NOTE: this will not work if task_id is currently WAITING on some list outside the scheduler!!!
    if (Task* t = scheduler.get_by_tid(task_id)) {
        block_current_task(t->wait_queue);
        return true;
    }
    return false;
}

/**
 * @brief   Take the current task out of the running queue and put it on "list"
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
//void TaskManager::dequeue_current_task(TaskList& list) {
//    KLockGuard lock;   // prevent reschedule
//
//    Task* current_task = scheduler.get_current_task();
//    list.push_front(current_task);
//    scheduler.remove(current_task);
//}

/**
 * @brief   Put the "task" back on running queue, or remove it if task group is ordered to be terminated
 * @note    TASK MUST HAVE BEEN FIRST ADDED AND INITIALIZED WITH "add_task"
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
//void TaskManager::enqueue_task_back(Task* task) {
//    KLockGuard lock;    // prevent reschedule
//
//    if (task->task_group_data->termination_pending)
//        wakeup_waitings_and_delete_task(task);
//    else
//        scheduler.add(task);
//}

/**
 * @brief   Block current task and put it on waiting "list"
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
void TaskManager::block_current_task(TaskList& list) {
    KLockGuard lock;   // prevent reschedule

    Task* current_task = scheduler.get_current_task();
    current_task->state = TaskState::BLOCKED;
    list.push_front(current_task);
}

/**
 * @brief   Unblock the tasks from waiting "list"
 * @note    TASK MUST HAVE BEEN FIRST ADDED AND INITIALIZED WITH "add_task"
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
void TaskManager::unblock_tasks(TaskList& list) {
    KLockGuard lock;   // prevent reschedule

    while (Task* t = list.pop_front()) {
        if (scheduler.is_valid_task(t)) // task might have been deleted while sleeping (exit_group)
            t->state = TaskState::RUNNING;
    }
}

/**
 * @brief   Choose next task to run and load its page table level4 into cr3
 * @note    Execution context: Interrupt only (on kill_current_task, schedule)
 * @note    There always need to be at least the "idle" task on the queue
 */
CpuState* TaskManager::pick_next_task_and_load_address_space() {
    Task* next_task = scheduler.pick_next_task();
    u64 pml4_physical_address = next_task->task_group_data->pml4_phys_addr;
    PageTables::load_address_space(pml4_physical_address);

    return next_task->cpu_state;


//    if (Task* next_task = scheduler.pick_next_task()) {
//        u64 pml4_physical_address = next_task->task_group_data->pml4_phys_addr;
//        PageTables::load_address_space(pml4_physical_address);
//        return next_task->cpu_state;
//    }
//    else {  // scheduler says "Not task eligible for schedule"
//        return idle_task; // boot_task?
//    }
}
} // namespace multitasking {
