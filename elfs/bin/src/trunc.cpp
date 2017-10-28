/**
 *   @file: trunc.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"

const u32 MAX_CHARS = 90;
char buff[MAX_CHARS];

const char ERROR_NO_INPUT[]         = "trunc: please specify filename and new file size in bytes\n";
const char ERROR_FILE_IS_DIR[]      = "trunc: specified filename points to a directory\n";
const char ERROR_FILE_NOT_EXISTS[]  = "trunc: specified file does not exist\n";
const char ERROR_TRUNC_ERROR[]      = "trunc: file truncate error\n";

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        print(ERROR_NO_INPUT);
        return 1;
    }

    const char* filename = argv[1];
    const char* newsizestr = argv[2];
    long newsize = str_to_long(newsizestr);

    struct stat s;
    if (syscalls::stat(filename, &s) < 0 ) {
        print(ERROR_FILE_NOT_EXISTS);
        return 1;
    }

    if (s.st_mode == S_IFDIR) {
        print(ERROR_FILE_IS_DIR);
        return 1;
    }

    if (syscalls::truncate(filename, newsize) < 0) {
        print(ERROR_TRUNC_ERROR);
        return 1;
    }

    return 0;
}


