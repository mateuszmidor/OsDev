/**
 *   @file: fibb.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "utils.h"

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
 * @return  Fibonacci of argv[1]
 */
int main(int argc, char* argv[]) {
    if (argc < 2)
        return -1;
    else
        return fib_rec(str_to_long(argv[1]));
}
