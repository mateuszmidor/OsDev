/**
 *   @file: tail.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "utils.h"

const u32 MAX_CHARS = 90;
char buff[MAX_CHARS];

const char ERROR_NO_INPUT_FILE[] = "Please specify filename.\n";
const char ERROR_FILE_IS_DIR[] = "Specified filename points to a directory\n";
const char ERROR_FILE_NOT_EXISTS[] = "Specified file does not exist";
const char ERROR_OPENING_FILE[] = "Error opening file";
const char ERROR_SEEK_ERROR[] = "File seek error";

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

    struct stat s;
    if (syscalls::stat(absolute_filename, &s) < 0 ) {
        print(ERROR_FILE_NOT_EXISTS);
        return -1;
    }

    if (s.st_mode == S_IFDIR) {
        print(ERROR_FILE_IS_DIR);
        return -1;
    }


    int fd = syscalls::open(absolute_filename);
    if (fd < 0) {
        print(ERROR_OPENING_FILE);
        return -1;
    }

    ssize_t total = 0;
    ssize_t count;
    u32 position = s.st_size > MAX_CHARS ? s.st_size - MAX_CHARS : 0;
    if (syscalls::lseek(fd, position, SEEK_SET) < 0) {
        print(ERROR_SEEK_ERROR);
        return -1;
    }

    print("\n");
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0) {
        print(buff, count);
        total += count;
    }

    syscalls::close(fd);
    return total;
}


