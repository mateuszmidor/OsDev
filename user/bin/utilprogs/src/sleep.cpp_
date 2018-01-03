/**
 *   @file: sleep.cpp
 *
 *   @date: Nov 14, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

using namespace ustd;

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    const auto SECOND = 1000*1000*1000;

    cout::print("Sleep demo 1...");
    syscalls::nsleep(SECOND);

    cout::print("2...");
    syscalls::nsleep(SECOND);

    cout::print("3...");
    syscalls::nsleep(SECOND);

    cout::print("4...");
    syscalls::nsleep(SECOND);

    cout::print("5...");
    syscalls::nsleep(SECOND);

    cout::print("done.");

    return 0;
}
