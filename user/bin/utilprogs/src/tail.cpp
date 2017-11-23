/**
 *   @file: tail.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

const u32 MAX_CHARS = 90;
char buff[MAX_CHARS];

const char ERROR_NO_INPUT_FILE[]    = "tail: please specify filename.\n";
const char ERROR_FILE_IS_DIR[]      = "tail: specified filename points to a directory\n";
const char ERROR_FILE_NOT_EXISTS[]  = "tail: specified file does not exist\n";
const char ERROR_OPENING_FILE[]     = "tail: cant open specified file\n";
const char ERROR_SEEK_ERROR[]       = "tail: file seek error\n";

using namespace ustd;

/**
 * @brief   Entry point
 * @return  Number of printed characters
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    const char* filename = argv[1];

    struct stat s;
    if (syscalls::stat(filename, &s) < 0 ) {
        cout::print(ERROR_FILE_NOT_EXISTS);
        return 1;
    }

    if (s.st_mode == S_IFDIR) {
        cout::print(ERROR_FILE_IS_DIR);
        return 1;
    }


    int fd = syscalls::open(filename);
    if (fd < 0) {
        cout::print(ERROR_OPENING_FILE);
        return 1;
    }

    ssize_t total = 0;
    ssize_t count;
    u32 position = s.st_size > MAX_CHARS ? s.st_size - MAX_CHARS : 0;
    if (syscalls::lseek(fd, position, SEEK_SET) < 0) {
        cout::print(ERROR_SEEK_ERROR);
        return 1;
    }

    cout::print("\n");
    while ((count = syscalls::read(fd, buff, sizeof(buff))) > 0) {
        cout::print(buff, count);
        total += count;
    }

    syscalls::close(fd);
    return total;
}


