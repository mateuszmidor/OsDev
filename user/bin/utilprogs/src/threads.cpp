/**
 *   @file: threads.cpp
 *
 *   @date: Nov 29, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

using namespace ustd;

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

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    auto t1 = syscalls::task_lightweight_run((unsigned long long)print_a, 55, "threads_print_a");
    syscalls::msleep(100);
    auto t2 = syscalls::task_lightweight_run((unsigned long long)print_b, 66, "threads_print_b");

    syscalls::task_wait(t1);
    syscalls::task_wait(t2);
    cout::print("\n Threading done.");

    return 0;
}
