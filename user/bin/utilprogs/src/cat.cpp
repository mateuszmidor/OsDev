/**
 *   @file: cat.cpp
 *
 *   @date: Oct 19, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"

char buff[1024*1024];
const char ERROR_NO_INPUT_FILE[]    = "cat: please specify file name\n";
const char ERROR_OPENING_FILE[]     = "cat: cant open specified file\n";


/**
 * @brief   Entry point
 * @return  Number of printed characters
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    const char* path = argv[1];

    int fd = syscalls::open(path);
    if (fd < 0) {
        print(ERROR_OPENING_FILE);
        return 1;
    }

    ssize_t total = 0;
    ssize_t count;
    print("\n");
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0) {
        print(buff, count);
        total += count;
    }

    syscalls::close(fd);
    return total;
}
