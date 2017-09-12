/**
 *   @file: square.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "utils.h"

/**
 * @brief   Beacuse this is a -nostdlib binary, we need to provide "_start" symbol for the system to know where to start execution
 */
__asm__ __volatile__(
    ".global _start     \n;"
    "_start:            \n;"
    "call main          \n;"
    "mov %rax, %rbx     \n;"
    "mov $1, %rax       \n;"
    "int $0x80          \n;"
);

/**
 * @brief   Entry point
 * @return  Sum of provided integer arguments
 */
int main(int argc, char* argv[]) {
    if (argc < 2)
        return -1;

    int result = 0;
    for (int i = 1; i < argc; i++)
        result += str_to_long(argv[i]);

    return result;
}
