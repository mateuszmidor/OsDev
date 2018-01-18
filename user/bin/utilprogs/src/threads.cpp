/**
 *   @file: threads.cpp
 *
 *   @date: Nov 29, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

using namespace cstd::ustd;

void print_a(unsigned ret) {
    for (unsigned i = 0; i < 40; i++) {
        cout::print("A");
        syscalls::msleep(200);
    }
    syscalls::exit(ret);
}

void print_b(unsigned ret) {
    for (unsigned i = 0; i < 40; i++) {
        cout::print("B");
        syscalls::msleep(200);
    }
    syscalls::exit(ret);
}

s64 run_thread(unsigned long long func, unsigned arg, const char name[]) {
    auto tid = syscalls::task_lightweight_run(func, arg, name);
    switch (tid) {
    case -ENOMEM:
        cout::print("threads: no memory to run thread. Exit.\n");
        break;

    case -EPERM:
        cout::print("threads: too many threads running aleady. Exit.\n");
        break;

    default:
        break;
    }
    return tid;
}
/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    auto t1 = run_thread((unsigned long long)print_a, 55, "threads_print_a");
    if (t1 < 0)
        return 1;

    syscalls::msleep(100);

    auto t2 = run_thread((unsigned long long)print_b, 66, "threads_print_b");
    if (t2 < 0)
        return 1;

    syscalls::task_wait(t1);
    syscalls::task_wait(t2);
    cout::print("\n Threading done.\n");

    return 0;
}
