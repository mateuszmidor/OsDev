/**
 *   @file: fibb.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "Cout.h"
#include "StringUtils.h"

using namespace cstd;
using namespace cstd::ustd;
/**
 * @brief   Recursive Fibonacci; tests stack well
 */
unsigned long long fib_rec(int n) {
    if(n <= 1) {
        return n;
    }
    return fib_rec(n-1) + fib_rec(n-2);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print("fibb: please provide number to calc fibonacci for\n");
        return 1;
    }

    auto result = fib_rec(StringUtils::to_int(argv[1]));
    cout::format("Fibonacci result: %\n", result);

    return 0;
}
