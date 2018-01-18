/**
 *   @file: ps.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

char buff[1024];
const char KERNEL_PROC_FILE[]   = "/proc/psinfo";
const char ERROR_CANT_OPEN[]    = "ps: cant open /proc/psinfo\n";

using namespace cstd::ustd;

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    int fd = syscalls::open(KERNEL_PROC_FILE);
    if (fd < 0) {
        cout::print(ERROR_CANT_OPEN);
        return 1;
    }

    ssize_t count;
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0)
        cout::print(buff, count);

    syscalls::close(fd);
    return 0;
}
