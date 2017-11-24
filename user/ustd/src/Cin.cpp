/**
 *   @file: Cin.cpp
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "Cin.h"

namespace ustd {

int cin::stdin_fd = -1;

string cin::readln() {
    if (stdin_fd < 0)
        stdin_fd = syscalls::open("/dev/stdin", 2);

    if (stdin_fd >= 0) {
        const size_t BUFF_SIZE = 256;
        char buff[BUFF_SIZE];
        syscalls::read(stdin_fd, buff, BUFF_SIZE);
        buff[BUFF_SIZE-1] = '\0';
        return buff;
    }

    return {};
}
} /* namespace ustd */
