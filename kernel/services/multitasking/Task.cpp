/**
 *   @file: Task.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Task.h"
#include "SysCallNumbers.h"

using namespace hardware;

namespace multitasking {


/**
 * Constructor.
 * Task stack is constructed as follows:
   0|FREE STACK|CpuState|TaskEpilogue|STACK_MAX
                        ^
                  here is rsp when first time jumping from interrupt to task function. So on return the function takes ret addr from TaskEpilogue.

  @param    entrypoint      Task main function address
  @param    arg1, arg2      Task main function param 1 and 2
  @param    stack_addr      Virtual address of task stack
  @param    stack_size      Task stack size in bytes

  @param    pml4_phys_addr  Page tables root (PHYSICAL ADDRESS). Defines task address space.
                            If != 0, make sure the entrypoint and stack will be accessible from this address space
  @param    cwd             current working directory
*/
Task::Task(TaskEntryPoint2 entrypoint, const char name[], u64 arg1, u64 arg2, bool user_space, u64 stack_addr, u64 stack_size, TaskGroupDataPtr task_group_data) :
        entrypoint(entrypoint),
        name(name),
        arg1(arg1), arg2(arg2),
        is_user_space(user_space),
        stack_addr(stack_addr), stack_size(stack_size),
        cpu_state((CpuState*)0xBAD), task_id(0), state(TaskState::RUNNING),
        task_group_data(task_group_data) {
}

/**
 * @brief   Cleanup task dynamic data
 */
Task::~Task() {
    // delete kernelspace stack. userspace stack is removed together with task address space
    if (!is_user_space)
        delete[] (u8*)stack_addr;
}
/**
 * @brief   Setup cpu state and return address on the task stack before running the task
 * @param   exitpoint Address of a function that the task should return to upon exit
 */
void Task::prepare(TaskId tid, TaskExitPoint exitpoint) {
    const u64 STACK_END = (u64)stack_addr + stack_size;

    // prepare task epilogue ie. where to return from task function
    TaskEpilogue* task_epilogue = (TaskEpilogue*)(STACK_END - sizeof(TaskEpilogue));
    new (task_epilogue) TaskEpilogue {(u64)exitpoint};

    // prepare task cpu state to setup cpu register with
    cpu_state = (CpuState*)(STACK_END - sizeof(CpuState) - sizeof(TaskEpilogue));
    new (cpu_state) CpuState {(u64)entrypoint, (u64)task_epilogue, arg1, arg2, is_user_space, task_group_data->address_space.pml4_phys_addr};

    task_id = tid;
}

bool Task::is_parent_of(const Task& t) const {
    return task_id == t.task_group_data->parent_task_id;
}

bool Task::is_in_group(const TaskGroupDataPtr& g) const {
    return task_group_data == g;
}

void Task::idle() {
    while (true)
        asm volatile("hlt");
}

void Task::yield() {
    msleep(0);
}

void Task::msleep(u64 milliseconds) {
    asm volatile("int $0x80" : : "a"(middlespace::Int80hSysCallNumbers::NANOSLEEP), "b"(milliseconds*1000*1000));
}

void Task::exit(u64 result_code) {
    asm volatile("int $0x80" : : "a"(middlespace::Int80hSysCallNumbers::EXIT), "b"(result_code));
}

void Task::exit_group(u64 result_code) {
    asm volatile("int $0x80" : : "a"(middlespace::Int80hSysCallNumbers::EXIT_GROUP), "b"(result_code));
}

/**
 * @brief   Send a signal to a task
 * @param   task_id Task to send a signal to
 * @param   signal  Signal to kill the task with, unused for now
 * @return  0 on success
 *          Negative error code on failure
 * @note    If task_id == current_task_id, this call never returns but another task is picked instead
 */
s32 Task::kill(TaskId task_id, s32 signal) {
    u32 result;
    asm volatile("int $0x80" : "=a"(result) : "a"(middlespace::Int80hSysCallNumbers::KILL), "b"(task_id), "c"(signal));
    return -result;
}
} /* namespace multitasking */
