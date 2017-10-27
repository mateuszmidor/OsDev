/**
 *   @file: df.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "ustd.h"
#include "utils.h"

char buff[1024];
const char KERNEL_PROC_FILE[]   = "/proc/mountinfo";
const char ERROR_CANT_OPEN[]    = "df: cant open /proc/mountinfo\n";


/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    int fd = syscalls::open(KERNEL_PROC_FILE);
    if (fd < 0) {
        print(ERROR_CANT_OPEN);
        return 1;
    }

    ssize_t count;
    print("\n");
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0)
        print(buff, count);

    syscalls::close(fd);
    return 0;
}


