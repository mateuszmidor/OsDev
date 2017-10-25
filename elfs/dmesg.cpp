/**
 *   @file: dmesg.cpp
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "ustd.h"
#include "utils.h"

char buff[1024*1024];
const char KERNEL_LOG_FILE[] = "/proc/kmsg";
const char ERROR_NO_INPUT_FILE[] = "/proc/kmsg doesnt exist";


/**
 * @brief   Entry point
 */
int main(int argc, char* argv[]) {
    int fd = syscalls::open(KERNEL_LOG_FILE);
    if (fd == -1) {
        print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    ssize_t count;
    print("\n");
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0)
        print(buff, count);

    syscalls::close(fd);
    return 0;
}
