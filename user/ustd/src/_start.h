/**
 *   @file: _start.h
 *
 *   @date: Sep 15, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS__START_H_
#define ELFS__START_H_

/**
 * @brief   Beacuse this is a -nostdlib binary, we need to provide "_start" symbol for the system to know where to start execution
 */
__asm__ __volatile__(
    ".global _start     ;"
    "_start:            ;"
    "call main          ;"
    "mov %rax, %rdi     ;"  // error code
    "mov $60, %rax      ;"  // syscall 60: exit
    "syscall            ;"
);


#endif /* ELFS__START_H_ */
