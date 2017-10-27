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
 * @return  0 on success, 1 on error
 */
char buff[12];
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print("fibb: please provide number to calc fibonacci for\n");
        return 1;
    }

    int result = fib_rec(str_to_long(argv[1]));

    long_to_str(result, 10, buff);
    print("Fibonacci result: ");
    print(buff);
    print("\n");
    return 0;
}
