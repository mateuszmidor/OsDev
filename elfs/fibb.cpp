/**
 *   @file: fibb.cpp
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
 * @brief   Recursive Fibonacci; tests stack well
 */
int fib_rec(int n) {
    if(n <= 1) {
        return n;
    }
    return fib_rec(n-1) + fib_rec(n-2);
}

/**
 * @brief   Entry point
 * @return  Return fibonacci of argc
 */
int main(int argc, char* argv[]) {
    return fib_rec(argc);
}
