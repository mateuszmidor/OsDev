/**
 *   @file: utils.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "utils.h"


extern "C" size_t strlen(char const * str) {
    size_t result = 0;
    while (str[result]) {
        result++;
    }

    return result;
}

void _print(int fd, const char str[], size_t len) {
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

int stdout_fd = -1;
void print(const char str[], size_t count) {
    if (stdout_fd < 0)
        stdout_fd = syscalls::open("/dev/stdout", 2);

    if (stdout_fd >= 0)
        _print(stdout_fd, str, count);
}

void print(const char str[]) {
   print(str, strlen(str));
}
