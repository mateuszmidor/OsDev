/**
 *   @file: rm.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

const char ERROR_NO_INPUT_FILE[]    = "rm: please provide directory path, optionally '-d'. Eg. rm -r DIR\n";
const char ERROR_DOESNT_EXIST[]     = "rm: given filename doesnt exist\n";
const char ERROR_DIRECTORY[]        = "rm: given filename points to a directory. Use '-d'\n";
const char ERROR_CANT_REMOVE_FILE[] = "rm: cant remove given file\n";
const char ERROR_CANT_REMOVE_DIR[]  = "rm: cant remove given directory\n";

using namespace cstd::ustd;


bool exists(const char filename[]) {
    struct stat s;
    return !(syscalls::stat(filename, &s) < 0);
}

bool is_directory(const char filename[]) {
    struct stat s;
    if (syscalls::stat(filename, &s) < 0) {
        return false;
    }

    return (s.st_mode == S_IFDIR);
}

int rm_file(const char path[]) {
    int result = syscalls::unlink(path);
    if (result < 0) {
        cout::print(ERROR_CANT_REMOVE_FILE);
        return 1;
    }

    return 0;
}

int rm_directory(const char path[]) {
    int result = syscalls::rmdir(path);
    if (result < 0) {
        cout::print(ERROR_CANT_REMOVE_DIR);
        return 1;
    }

    return 0;
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    const char* flags = "";
    const char* path;

    if (argc == 2)
        path = argv[1];
    else {
        flags = argv[1];
        path = argv[2];
    }

    if (!exists(path)) {
        cout::print(ERROR_DOESNT_EXIST);
        return 1;
    }

    if (is_directory(path)){
        if (flags[1] != 'd') {
            cout::print(ERROR_DIRECTORY);
            return 1;
        }
        return rm_directory(path);
    } else
        return rm_file(path);
}
