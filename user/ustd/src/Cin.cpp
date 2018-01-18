/**
 *   @file: Cin.cpp
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "Cin.h"

namespace cstd {
namespace ustd {

int cin::stdin_fd = -1;

string cin::readln() {
    if (stdin_fd < 0)
        stdin_fd = syscalls::open("/dev/stdin", 2);

    if (stdin_fd < 0)
        return {};


    const u32 BUFF_SIZE = 256;
    char buff[BUFF_SIZE];

    ssize_t count = syscalls::read(stdin_fd, buff, BUFF_SIZE - 1);
    if (count > 0)
        return string(buff, count-1); // skip ending \n
    else
        return {};
}

} /* namespace ustd */
} /* namespace cstd */
