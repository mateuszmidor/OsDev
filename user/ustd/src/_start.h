/**
 *   @file: _start.h
 *
 *   @date: Sep 15, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS__START_H_
#define ELFS__START_H_

typedef void (*Constructor)();
extern "C" Constructor start_ctors;
extern "C" Constructor end_ctors;

/**
 * @brief   Call the global constructors
 * @note    start_ctors and end_ctors defined in linker.ld
 */
extern "C" void run_ctors() {
    for (Constructor* c = &start_ctors; c != &end_ctors; c++)
        (*c)();
}

/**
 * @brief   Beacuse this is a -nostdlib binary, we need to provide "_start" symbol for the system to know where to start execution
 */
__asm__ __volatile__(
    ".global _start     ;"
    "_start:            ;"
    "push %rsi          ;"
    "push %rdi          ;"
    "call run_ctors     ;"
    "pop %rdi           ;"
    "pop %rsi           ;"
    "call main          ;"
    "mov %rax, %rdi     ;"  // error code
    "mov $231, %rax     ;"  // syscall 231: exit_group
    "syscall            ;"
);

#endif /* ELFS__START_H_ */
