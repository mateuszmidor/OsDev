/**
 *   @file: cat.cpp
 *
 *   @date: Oct 19, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"

char buff[1024*1024];
const char ERROR_NO_INPUT_FILE[] = "Please specify file name.\n";
const char ERROR_INVALID_FILE[] = "Specified file is directory or not even exists.\n";


/**
 * @brief   Entry point
 * @return  Simply return size of given file
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    const char* absolute_filename = argv[1];

    auto fd = syscalls::open(absolute_filename);
    if (fd == -1) {
        print(ERROR_INVALID_FILE);
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
