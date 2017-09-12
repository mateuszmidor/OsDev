/**
 *   @file: ret3.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

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
 * @return  Simply return number 3
 */
int main() {
    return 3;
}
