/**
 *   @file: Cout.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "Cout.h"

namespace ustd {

int cout::stdout_fd = -1;


void cout::print(const string& s) {
    print(s.c_str(), s.length());
}

void cout::print(const char str[], size_t count) {
    if (stdout_fd < 0)
        stdout_fd = syscalls::open("/dev/stdout", 2);

    if (stdout_fd >= 0)
        _print(stdout_fd, str, count);
}

void cout::_print(int fd, const char str[], size_t len) {
    while (len > 0) {
        ssize_t written = syscalls::write(fd, str, len);

        if (written < 0)
            break;
        else {
            str += written;
            len -= written;
        }
    }
}

} /* namespace ustd */
