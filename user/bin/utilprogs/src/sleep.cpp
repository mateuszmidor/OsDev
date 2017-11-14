/**
 *   @file: sleep.cpp
 *
 *   @date: Nov 14, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "utils.h"


/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    const auto SECOND = 1000*1000*1000;
    print("Sleep demo 1...");
    syscalls::nsleep(SECOND);

    print("2...");
    syscalls::nsleep(SECOND);

    print("3...");
    syscalls::nsleep(SECOND);

    print("4...");
    syscalls::nsleep(SECOND);

    print("5...");
    syscalls::nsleep(SECOND);

    print("done.");

    return 0;
}



